/*
 * rules.c
 *
 *  Created on: 05.05.2015
 *      Author: zaphod
 */

#include "rules.h"

typedef struct {
	uint8_t active;		// 1 for active, 0 for inactive
	char type;				// H for humidity, T for temperature
	uint8_t sensor;		// 1-2 for two sensors
	uint8_t lower;		// 0-99 - the desired value
	uint8_t upper;		// 0-99 - the desired value
} Rule;

Rule rules[RULES_COUNT];

void rules_setup(void) {
	// load rule set from eeprom
	persistence_restore();
}

void rules_execute_rule(uint8_t rule_number) {
	if (rule_number > 0 && rule_number <= RULES_COUNT) {
		Rule rule = rules[rule_number - 1];

		// do nothing for an inactive rule. the correspondig socket may be controlled
		// manually using the on/off commands
		if (!rule.active) {
			return;
		}

		Sensor_Reading reading = sensors_read_sensor(rule.sensor);

		if(!reading.error) {
			uint8_t sensor_value;
			bool state = relay_state(rule_number);

			switch (toupper(rule.type)) {
				case 'T':
					sensor_value = reading.temperature;
					break;
				case 'H':
					sensor_value = reading.humidity;
					break;
				default:
					return;
			}

			// this logic assumes that switching the relay on will RAISE the sensor value
			// eventually. that means that it only works if we have a heater and a humidifier.
			// if one would attach a cooler and a desiccator, things would go horribly wrong.
			bool result = sensor_value <= (state ? rule.upper : rule.lower);

			if (result && !state) {
				// we want the relay on, but it isn't
				relay_on(rule_number);
			} else if (!result && state) {
				// it is on but it shouldn't be
				relay_off(rule_number);
			}
		}
		else {
			// always switch the relay off if there was an error.
			relay_off(rule_number);
		}
	}
}

void rules_execute(void) {
	for (uint8_t rule_number = 1; rule_number <= RULES_COUNT; rule_number++) {
		rules_execute_rule(rule_number);
	}
}

int rules_set_rule_internal(uint8_t rule_number, char* rule, uint8_t persist) {
	if (rule_number > 0 && rule_number <= RULES_COUNT) {
		while (*rule == ' ')
			rule++;

		if (strlen(rule) >= 3 && strlen(rule) <= 8) {
			char type = toupper(*rule++);
			long int sensor = strtol(rule, &rule, 10);
			while (*rule == ' ')
				rule++;
			long int lower = strtol(rule, &rule, 10);
			while (*rule == ' ')
				rule++;
			long int upper = strtol(rule, &rule, 10);

			if ((type == 'H' || type == 'T') && sensor > 0 && sensor <= SENSORS_COUNT && upper >= 0 && upper <= 99 && lower >= 0 && lower <= 99 && lower <= upper) {
				rules[rule_number - 1].type = type;
				rules[rule_number - 1].sensor = sensor;
				rules[rule_number - 1].lower = lower;
				rules[rule_number - 1].upper = upper;
				rules[rule_number - 1].active = 1;

				rules_execute_rule(rule_number);

				if (persist) {
					persistence_persist();
				}

				return 0;
			}
		} else if (strlen(rule) == 0) {
			rules[rule_number - 1].active = 0;

			relay_off(rule_number);

			if (persist) {
				persistence_persist();
			}

			return 0;
		}

		return RULES_ERR_FORMAT;
	}

	return RULES_ERR_INDEX;
}

int rules_set_rule(uint8_t rule_number, char* rule) {
	return rules_set_rule_internal(rule_number, rule, 1);
}

int rules_set_rule_volatile(uint8_t rule_number, char* rule) {
	return rules_set_rule_internal(rule_number, rule, 0);
}

int rules_print_rule(uint8_t rule_number, char* buffer) {
	buffer[0] = 0;

	if (rule_number > 0 && rule_number <= RULES_COUNT) {
		if (!rules[rule_number - 1].active) {
			buffer[0] = 0;
			return RULES_ERR_INACTIVE;
		}

		sprintf(buffer, "%c%d %02d %02d", rules[rule_number - 1].type, rules[rule_number - 1].sensor, rules[rule_number - 1].lower, rules[rule_number - 1].upper);
		return 0;
	} else {
		return RULES_ERR_INDEX;
	}
}

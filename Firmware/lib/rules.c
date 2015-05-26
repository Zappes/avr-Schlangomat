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
		uint8_t value;		// 0-99 - the desired value
} Rule;

Rule rules[RULES_COUNT];

// hysteresis value for humidity
uint8_t rules_hysteresis_humidity = 3;

// hysteresis value for temperature
uint8_t rules_hysteresis_temperature = 3;

void rules_setup(void) {
	#ifdef DEBUG_ENABLED
		rules_set_rule(1, "t1 18");
		rules_set_rule(2, "h1 50");
		rules_set_rule(3, "t2 24");
		rules_set_rule(4, "h2 30");
	#endif
}

int rules_set_rule(uint8_t rule_number, char* rule) {
	if(rule_number > 0 && rule_number <= RULES_COUNT) {
		while(rule[0] == ' ')
			rule++;

		if(strlen(rule) == 5) {
			int type = toupper(rule[0]);
			uint8_t sensor = atoi(rule + 1);
			uint8_t value = atoi(rule + 3);		// 00-99 - the value to compare to

			if((type == 'H' || type == 'T') &&
					sensor > 0 && sensor <= SENSORS_COUNT &&
					value >= 0 && value <= 99) {
				rules[rule_number-1].type = type;
				rules[rule_number-1].sensor = sensor;
				rules[rule_number-1].value = value;
				rules[rule_number-1].active = 1;

				return 0;
			}
		}
		else if(strlen(rule) == 0) {
			rules[rule_number-1].active = 0;

			return 0;
		}

		return RULES_ERR_FORMAT;
	}

	return RULES_ERR_INDEX;
}

int rules_print_rule(uint8_t rule_number, char* buffer) {
	buffer[0] = 0;

	if(rule_number > 0 && rule_number <= RULES_COUNT) {
		if(!rules[rule_number-1].active) {
			return RULES_ERR_INACTIVE;
		}

		sprintf(buffer, "%c%d %02d", rules[rule_number-1].type, rules[rule_number-1].sensor, rules[rule_number-1].value);
		return 0;
	}
	else {
		return RULES_ERR_INDEX;
	}
}

void rules_execute_rule(uint8_t rule_number) {
	if(rule_number > 0 && rule_number <= RULES_COUNT) {
		Rule rule = rules[rule_number-1];
		Sensor_Reading reading = sensors_read_sensor(rule.sensor);
		uint8_t sensor_value;
		uint8_t hysteresis;
		bool state = relay_state(rule_number);

		switch(rule.type) {
			case 'T':
			case 't':
				sensor_value = reading.temperature;
				hysteresis = rules_hysteresis_temperature;
				break;
			case 'H':
			case 'h':
				sensor_value = reading.humidity;
				hysteresis = rules_hysteresis_humidity;
				break;
		}

		uint8_t upper = rule.value + hysteresis;
		if(upper > 99) upper = 99;

		uint8_t lower = rule.value > hysteresis ? rule.value - hysteresis : 0;

		// this logic assumes that switching the relay on will RAISE the sensor value
		// eventually. that means that it only works if we have a heater and a humidifier.
		// if one would attach a cooler and a desiccator, things would go horribly wrong.
		bool result = sensor_value <=  (state ? upper : lower);

		DEBUGF("Rule %d: %c%d %02d", rule_number, rule.type, rule.sensor, rule.value)
		DEBUGF("  Sensor value: %02d", sensor_value)
		DEBUGF("  Rule value:   %02d", rule.value)
		DEBUGF("  Hysteresis:   %02d", hysteresis)
		DEBUGF("  Lower limit:  %02d", lower)
		DEBUGF("  Upper limit:  %02d", upper)
		DEBUGF("  Relay state:  %s", state ? "on" : "off")
		DEBUGF("  Result:       %s", result ? "on" : "off")

		if(result && !state) {
			// we want the relay on, but it isn't
			relay_on(rule_number);
		}
		else if(!result && state) {
			// it is on but it shouldn't be
			relay_off(rule_number);
		}
	}
}

void rules_execute(void) {
	DEBUG("======================================================================================")
	for(uint8_t rule_number = 1; rule_number <= RULES_COUNT; rule_number++) {
		rules_execute_rule(rule_number);
	}
	DEBUG("======================================================================================\r\n")
}


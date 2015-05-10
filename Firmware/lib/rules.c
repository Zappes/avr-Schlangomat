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
		char operator;		// either < or >
		uint8_t value;		// 0-99 - the value to compare to
} Rule;

Rule rules[4];

int rules_set_rule(uint8_t rule_number, char* rule) {
	if(rule_number < RULES_COUNT) {
		while(rule[0] == ' ')
			rule++;

		if(strlen(rule) == 7) {
			int type = toupper(rule[0]);
			uint8_t sensor = atoi(rule + 1);
			char operator = rule[3];
			uint8_t value = atoi(rule + 5);		// 00-99 - the value to compare to

			if((type == 'H' || type == 'T') &&
					sensor > 0 && sensor < SENSORS_COUNT &&
					(operator == '<' || operator == '>') &&
					value >= 0 && value <= 99) {
				rules[rule_number].type = type;
				rules[rule_number].sensor = sensor;
				rules[rule_number].operator = operator;
				rules[rule_number].value = value;
				rules[rule_number].active = 1;

				return 0;
			}
		}

		return RULES_ERR_FORMAT;
	}

	return RULES_ERR_INDEX;
}

int rules_print_rule(uint8_t rule_number, char* buffer) {
	buffer[0] = 0;

	if(rule_number < RULES_COUNT) {
		if(!rules[rule_number].active) {
			return RULES_ERR_INACTIVE;
		}

		sprintf(buffer, "%c%d %c %02d", rules[rule_number].type, rules[rule_number].sensor, rules[rule_number].operator, rules[rule_number].value);
		return 0;
	}
	else {
		return RULES_ERR_INDEX;
	}
}

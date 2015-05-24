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

Rule rules[RULES_COUNT];

void rules_setup(void) {
	#ifdef DEBUG_ENABLED
		rules_set_rule(1, "t1 < 18");
		rules_set_rule(2, "h1 < 50");
		rules_set_rule(3, "t2 > 12");
		rules_set_rule(4, "h2 > 30");
	#endif
}

int rules_set_rule(uint8_t rule_number, char* rule) {
	if(rule_number > 0 && rule_number <= RULES_COUNT) {
		while(rule[0] == ' ')
			rule++;

		if(strlen(rule) == 7) {
			int type = toupper(rule[0]);
			uint8_t sensor = atoi(rule + 1);
			char operator = rule[3];
			uint8_t value = atoi(rule + 5);		// 00-99 - the value to compare to

			if((type == 'H' || type == 'T') &&
					sensor > 0 && sensor <= SENSORS_COUNT &&
					(operator == '<' || operator == '>') &&
					value >= 0 && value <= 99) {
				rules[rule_number-1].type = type;
				rules[rule_number-1].sensor = sensor;
				rules[rule_number-1].operator = operator;
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

		sprintf(buffer, "%c%d %c %02d", rules[rule_number-1].type, rules[rule_number-1].sensor, rules[rule_number-1].operator, rules[rule_number-1].value);
		return 0;
	}
	else {
		return RULES_ERR_INDEX;
	}
}

void rules_execute_rule(uint8_t rule_number) {
	if(rule_number > 0 && rule_number <= RULES_COUNT) {
		Rule rule = rules[rule_number-1];

		if(rule.active) {
			DEBUGF("Executing rule %d.", rule_number)
		}
		else {
			DEBUGF("Rule %d not active", rule_number)
		}
	}
}

void rules_execute(void) {
	for(uint8_t rule_number = 1; rule_number <= RULES_COUNT; rule_number++) {
		rules_execute_rule(rule_number);
	}
}


/*
 * rules.h
 *
 *  Created on: 05.05.2015
 *      Author: zaphod
 */

#ifndef RULES_H_
#define RULES_H_

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include "sensors.h"
#include "relay.h"
#include "debug.h"
#include "persistence.h"

#define RULES_COUNT 4

#define RULES_ERR_INDEX 1
#define RULES_ERR_FORMAT 2
#define RULES_ERR_INACTIVE 3

/**
 * Loads persisted rules from the EEPROM.
 */
void rules_setup(void);

/**
 * Sets the rule and persists the rule set afterwards.
 */
int rules_set_rule(uint8_t rule_number, char* rule);

/**
 * Sets the rule without persisting afterwards.
 */
int rules_set_rule_volatile(uint8_t rule_number, char* rule);

/**
 * Writes the string representation of the rule the buffer. Make sure to have at least 9 bytes of space there.
 */
int rules_print_rule(uint8_t rule_number, char* buffer);

/**
 * Executes the active rules.
 */
void rules_execute(void);

#endif /* RULES_H_ */

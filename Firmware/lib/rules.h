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

#include "sensors.h"

#define RULES_COUNT 4

#define RULES_ERR_INDEX 1
#define RULES_ERR_FORMAT 2
#define RULES_ERR_INACTIVE 3


int rules_set_rule(uint8_t rule_number, char* rule);
int rules_print_rule(uint8_t rule_number, char* buffer);

#endif /* RULES_H_ */

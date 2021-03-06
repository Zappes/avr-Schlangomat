/*
 * parser.c
 *
 *  Created on: 03.05.2015
 *      Author: zaphod
 */

#include "parser.h"

int get_num_from_param(char* param_buffer, int max) {
	long int num = 0;

	if (strlen(param_buffer) > 0) {
		num = strtol(param_buffer, 0, 10);
		if (num > max || num < 0)
			num = 0;
	}

	return (int)num;
}

bool is_command(char* buffer, char* command) {
	for (int i = 0; i < strlen(command); i++) {
		if (toupper(buffer[i]) != toupper(command[i])) {
			return false;
		}
	}

	return true;
}


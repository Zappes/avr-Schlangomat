/*
 * parser.c
 *
 *  Created on: 03.05.2015
 *      Author: zaphod
 */

#include "parser.h"

int get_num_from_param(char* param_buffer, int max) {
	int relay_num = 0;

	if (strlen(param_buffer) > 0) {
		relay_num = atoi(param_buffer);
		if (relay_num > max || relay_num < 0)
			relay_num = 0;
	}

	return relay_num;
}

bool is_command(char* buffer, char* command) {
	for (int i = 0; i < strlen(command); i++) {
		if (toupper(buffer[i]) != toupper(command[i])) {
			return false;
		}
	}

	return true;
}


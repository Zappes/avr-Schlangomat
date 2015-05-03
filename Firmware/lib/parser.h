/*
 * parser.h
 *
 *  Created on: 03.05.2015
 *      Author: zaphod
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>

int get_num_from_param(char* param_buffer, int max);
bool is_command(char* buffer, char* command);

#endif /* PARSER_H_ */

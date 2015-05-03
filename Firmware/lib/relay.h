/*
 * relay.h
 *
 *  Created on: 03.05.2015
 *      Author: zaphod
 */

#ifndef RELAY_H_
#define RELAY_H_

#include <avr/io.h>
#include <stdbool.h>

#define RELAY_DDR		DDRB
#define RELAY_PORT	PORTB
#define RELAY_PIN1	PB4
#define RELAY_PIN2	PB5
#define RELAY_PIN3	PB6
#define RELAY_PIN4	PB7

#define RELAY_COUNT 4

void relay_setup(void);
void relay_on(uint8_t relay_number);
void relay_off(uint8_t relay_number);
bool relay_state(uint8_t relay_number);

#endif /* RELAY_H_ */

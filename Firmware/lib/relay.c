/*
 * relay.c
 *
 *  Created on: 03.05.2015
 *      Author: zaphod
 */
#include "relay.h"

uint8_t relay_relaypins[] = { _BV(RELAY_PIN1) | _BV(RELAY_PIN2) | _BV(RELAY_PIN3) | _BV(RELAY_PIN4), _BV(RELAY_PIN1), _BV(RELAY_PIN2), _BV(RELAY_PIN3), _BV(RELAY_PIN4) };

void relay_setup(void) {
	RELAY_DDR |= relay_relaypins[0];
}

void relay_on(uint8_t relay_number) {
	if (relay_number >= 0 && relay_number <= RELAY_COUNT) {
		RELAY_PORT |= relay_relaypins[relay_number];
	}
}

void relay_off(uint8_t relay_number) {
	if (relay_number >= 0 && relay_number <= RELAY_COUNT) {
		RELAY_PORT &= ~(relay_relaypins[relay_number]);
	}
}

bool relay_state(uint8_t relay_number) {
	return (relay_number > 0 && relay_number <= RELAY_COUNT) && (RELAY_PORT & relay_relaypins[relay_number]) != 0;
}

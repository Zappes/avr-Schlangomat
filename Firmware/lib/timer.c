/*
 * timer.c
 *
 *  Created on: 24.05.2015
 *      Author: zaphod
 */

#include "timer.h"
volatile uint8_t timer_ready_flag = 0;
uint8_t timer_scale_limit = 5;
volatile uint8_t timer_scale_current = 0;

void timer_setup(uint8_t scale_limit) {
	// we have no pwm, which is the default mode of operation. using a 16 bit counter
	// and prescale with 256 results in roughly 1 interrupt per second.
	TCCR1B |= _BV(CS12);	// prescaler /256
	TIMSK1 |= _BV(TOIE1);	// enable overflow interrupt

	timer_scale_limit = scale_limit;
}

void timer_set_scale_limit(uint8_t scale_limit) {
	timer_scale_limit = scale_limit;
}

uint8_t timer_get_scale_limit(void) {
	return timer_scale_limit;
}

void timer_done(void) {
	timer_ready_flag = 0;
}

uint8_t timer_ready(void) {
	return timer_ready_flag;
}

ISR(TIMER1_OVF_vect) {
	if(timer_scale_current >= timer_scale_limit) {
		timer_ready_flag = 1;
		timer_scale_current = 0;
	}
	else {
		timer_scale_current++;
	}
}

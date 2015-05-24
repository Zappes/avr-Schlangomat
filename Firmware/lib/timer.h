/*
 * timer.h
 *
 *  Created on: 24.05.2015
 *      Author: zaphod
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

void timer_setup(uint8_t scale_limit);
uint8_t timer_ready(void);
void timer_done(void);

void timer_set_scale_limit(uint8_t scale_limit);
uint8_t timer_get_scale_limit(void);

#endif /* TIMER_H_ */

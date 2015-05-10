/*
 * sensors.h
 *
 *  Created on: 05.05.2015
 *      Author: zaphod
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <avr/io.h>
#include "am2302.h"

#define SENSORS_PIN1 PD6
#define SENSORS_PIN2 PD7
#define SENSORS_COUNT 2

#define SENSORS_ERR_INDEX 8;

typedef struct {
		uint8_t temperature;
		uint8_t temperature_frac;
		uint8_t humidity;
		uint8_t humidity_frac;
} sensor_reading;

int sensors_read_rensor(uint8_t sensor, sensor_reading* result);

#endif /* SENSORS_H_ */

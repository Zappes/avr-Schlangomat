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

#define SENSORS_ERR_INDEX 8

typedef struct {
		uint8_t error;
		uint8_t error_count;
		uint8_t temperature;
		uint8_t temperature_frac;
		uint8_t humidity;
		uint8_t humidity_frac;
} Sensor_Reading;

void sensors_setup(void);
void sensors_update_sensor(uint8_t sensor);
Sensor_Reading sensors_read_sensor(uint8_t sensor);

#endif /* SENSORS_H_ */

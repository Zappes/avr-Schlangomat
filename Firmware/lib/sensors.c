/*
 * sensors.c
 *
 *  Created on: 05.05.2015
 *      Author: zaphod
 */

#include "sensors.h"

uint8_t sensors_sensorpins[] = { SENSORS_PIN1, SENSORS_PIN2 };
Sensor_Reading sensors_readings[SENSORS_COUNT];
Sensor_Reading sensors_error = {SENSORS_ERR_INDEX, 1, 0 , 0, 0};

void sensors_setup(void) {
	sensors_update_sensor(0);
}

Sensor_Reading sensors_read_sensor(uint8_t sensor) {
	if (sensor > 0 && sensor <= SENSORS_COUNT) {
		return sensors_readings[sensor - 1];
	}
	else {
		return sensors_error;
	}
}

void sensors_update_sensor(uint8_t sensor) {
	if(sensor == 0) {
		for (uint8_t i = 1; i <= SENSORS_COUNT; i++) {
			sensors_update_sensor(i);
		}
	}
	if (sensor > 0 && sensor <= SENSORS_COUNT) {
		uint16_t tmp, hum = 0;
		uint8_t err = 0;

		err = am2302_read(&hum, &tmp, sensors_sensorpins[sensor - 1]);

		if (err) {
			sensors_readings[sensor-1].error = err;
			if(sensors_readings[sensor-1].error_count < 0xFF) {
				sensors_readings[sensor-1].error_count++;
			}
		} else {
			sensors_readings[sensor-1].error = 0;
			sensors_readings[sensor-1].error_count = 0;
			sensors_readings[sensor-1].temperature = tmp / 10;
			sensors_readings[sensor-1].temperature_frac = tmp % 10;
			sensors_readings[sensor-1].humidity = hum / 10;
			sensors_readings[sensor-1].humidity_frac = hum % 10;
		}
	}
}

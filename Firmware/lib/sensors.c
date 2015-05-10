/*
 * sensors.c
 *
 *  Created on: 05.05.2015
 *      Author: zaphod
 */

#include "sensors.h"

uint8_t sensors_sensorpins[] = { SENSORS_PIN1, SENSORS_PIN2 };

int sensors_read_rensor(uint8_t sensor, sensor_reading* result) {
	if (sensor > 0 && sensor <= SENSORS_COUNT) {
		uint16_t tmp, hum = 0;
		uint8_t err = 0;

		err = am2302_read(&hum, &tmp, sensors_sensorpins[sensor - 1]);

		if (err) {
			return err;
		} else {
			result->temperature = tmp / 10;
			result->temperature_frac = tmp % 10;
			result->humidity = hum / 10;
			result->humidity_frac = hum % 10;

			return 0;
		}
	} else {
		return SENSORS_ERR_INDEX;
	}

}

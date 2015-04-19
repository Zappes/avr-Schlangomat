/*
 * am2302.h
 *
 * Created on: 13.03.2013
 *     Author: Pascal Gollor
 *        web: http://www.pgollor.de
 *
 * Dieses Werk ist unter einer Creative Commons Lizenz vom Typ
 * Namensnennung - Nicht-kommerziell - Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugänglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-nc-sa/3.0/de/ oder wenden Sie sich
 * brieflich an Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 *
 *
 *
 *	AM2302/DHT22 (Temperatur und Luftfeuchtigkeitssensor)
 *  _________
 * |  -+-+-  |
 * | +-+-+-+ |
 * | +-+-+-+ |
 * |  -+-+-  |
 * | +-+-+-+ |
 * |_________|
 *   | | | |
 *   1 2 3 4
 *
 * 1. VCC (3 to 5V power)
 * 2. Data out (Pullup 4,7k)
 * 3. Not connected
 * 4. Ground
 *
 *
 * Error codes
 * 1: sensor sda bus not free
 * 2: timeout: bus master release time
 * 3: timeout: response to low time
 * 4: timeout: response to high time
 * 5: timeout: signal low timeout
 * 6: timeout: signal high timeout
 *
 * Read the datasheet for more information about the times.
 */


#include "am2302.h"

#include <avr/io.h>
#include <util/delay.h>

#define SENSOR_sda_out		DDR_SENSOR |= (1 << SENSOR)
#define SENSOR_sda_in			DDR_SENSOR &= ~(1 << SENSOR) // release sda => hi in consequence of pullup
#define SENSOR_sda_low    PIN_SENSOR &= ~(1 << SENSOR)
#define SENSOR_is_hi			PIN_SENSOR & (1 << SENSOR)
#define SENSOR_is_low		!(PIN_SENSOR & (1 << SENSOR))


uint8_t am2302(uint16_t *humidity, uint16_t *temp)
{
	if (SENSOR_is_low)
	{
		// bus not free
		return 1;
	}

	SENSOR_sda_out;
	SENSOR_sda_low;	// MCU start signal
	_delay_ms(20);	// start signal (pull sda down for min 0.8ms and maximum 20ms)
	SENSOR_sda_in;

	// Bus master has released time min: 20us, typ: 30us, max: 200us
	uint8_t timeout = 200;
	while(SENSOR_is_hi) {_delay_us(1); if (!timeout--) {return 2;}}

	// AM2302 response signal min: 75us typ:80us max:85us
	timeout = 85;
	while(SENSOR_is_low) {_delay_us(1); if (!timeout--) {return 3;}}  // response to low time
	timeout = 85;
	while(SENSOR_is_hi) {_delay_us(1); if (!timeout--) {return 4;}}  // response to high time


	/*
	 *            time in us: min typ max
	 *    signal 0 high time: 22  26  30     (bit=0)
	 *    signal 1 high time: 68  70  75     (bit=1)
	 *  signal 0,1 down time: 48  50  55
	 */

	uint8_t sensor_data[5]={0};
	for(uint8_t i = 0; i < 5; i++)
	{
		uint8_t sensor_byte = 0;
		for(uint8_t j = 1; j <= 8; j++) // get 8 bits from sensor
		{
			timeout = 55;
			while(SENSOR_is_low) {_delay_us(1); if (!timeout--) {return 5;}} // signal "0", "1" low time
			_delay_us(30);
			sensor_byte <<= 1; // add new lower byte
			if (SENSOR_is_hi) // if sda high after 30us => bit=1 else bit=0
			{
				sensor_byte |= 1;
				timeout = 45;  // 30us - 75us = 45us
				while(SENSOR_is_hi) {_delay_us(1); if (!timeout--) {return 6;}}
			}
		}
		sensor_data[i] = sensor_byte;
	}

	// checksum
	if ( ((sensor_data[0]+sensor_data[1]+sensor_data[2]+sensor_data[3]) & 0xff ) != sensor_data[4])
	{
		// debug output
		//printf("%b %b %b %b %b %b" CR, sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3], sensor_data[4], ((sensor_data[0]+sensor_data[1]+sensor_data[2]+sensor_data[3]) & 0xff ));
		return 7;
	}

	*humidity = (sensor_data[0]<<8) + sensor_data[1];
	*temp = (sensor_data[2]<<8) + sensor_data[3];

	return 0;
}

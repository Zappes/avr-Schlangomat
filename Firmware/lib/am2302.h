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
 */

#ifndef AM2302_H_
#define AM2302_H_


#include <avr/io.h>

#define DDR_SENSOR   DDRC
#define PORT_SENSOR  PORTC
#define PIN_SENSOR   PINC
#define SENSOR       PC0


uint8_t am2302(uint16_t *humidity, uint16_t *temp);



#endif /* AM2302_H_ */

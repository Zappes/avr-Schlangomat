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
 * Angepasst durch Gregor Ottmann, http://elektroschrott.bluephod.net, 19.04.2015
 *
 * Fest per Define vorgegebene Pin-Definition für den Sensor zu einem Funktionsparameter gemacht,
 * so dass mehr als ein Sensor (am gleichen Port) verwendet werden kann.
 */

#ifndef AM2302_H_
#define AM2302_H_


#include <avr/io.h>

#define DDR_SENSOR   DDRD
#define PORT_SENSOR  PORTD
#define PIN_SENSOR   PIND

uint8_t am2302_read(uint16_t *humidity, uint16_t *temp, uint8_t sensor);



#endif /* AM2302_H_ */

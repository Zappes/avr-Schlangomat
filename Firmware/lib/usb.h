/*
 * usb.h
 *
 *  Created on: 16.04.2015
 *      Author: zaphod
 */

#ifndef USB_H_
#define USB_H_

/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#include "Descriptors.h"

#include "LUFA/Drivers/USB/USB.h"
#include "LUFA/Platform/Platform.h"

#define USB_INPUT_BUFFER_SIZE 64
#define USB_FORMAT_BUFFER_SIZE 96

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

typedef void (*usb_callback_t)(char* commandBuffer);

/*
 * Sets the callback reference for buffer ready events
 */
usb_callback_t usb_set_callback(usb_callback_t cb);
void usb_setup(void);
void usb_read_loop(void);

/*
 * Writes a string/char[] to the serial port.
 */
void usb_write_string(char *str);

/*
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void usb_write_formatted(const char* format, ...);

/*
 * Writes a string/char[] to the serial port.
 */
void usb_writeln_string(char *str);

/*
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void usb_writeln_formatted(const char* format, ...);




void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

#endif /* USB_H_ */

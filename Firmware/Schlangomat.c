/*
 LUFA Library
 Copyright (C) Dean Camera, 2014.

 dean [at] fourwalledcubicle [dot] com
 www.lufa-lib.org
 */

/*
 Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

 Permission to use, copy, modify, distribute, and sell this
 software and its documentation for any purpose is hereby granted
 without fee, provided that the above copyright notice appear in
 all copies and that both that the copyright notice and this
 permission notice and warranty disclaimer appear in supporting
 documentation, and that the name of the author not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.

 The author disclaims all warranties with regard to this
 software, including all implied warranties of merchantability
 and fitness.  In no event shall the author be liable for any
 special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether
 in an action of contract, negligence or other tortious action,
 arising out of or in connection with the use or performance of
 this software.
 */

/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Schlangomat.h"

#define OUTPUT_BUFFER_SIZE 96
#define UART_BUFFER_SIZE 64
#define USB_BUFFER_SIZE 64

volatile char uart_buffer[UART_BUFFER_SIZE] = {0};
volatile uint8_t uart_ready = 0;
volatile char usb_buffer[USB_BUFFER_SIZE] = {0};
volatile uint8_t usb_ready = 0;

void handle_uart(char* buffer) {
	for(int i = 0; i < UART_BUFFER_SIZE - 1; i++) {
		uart_buffer[i] = buffer[i];

		if(buffer[i] == 0)
			break;
	}

	uart_buffer[UART_BUFFER_SIZE -1] = 0;
	uart_ready = 1;
}

void handle_usb(char* buffer) {
	for(int i = 0; i < USB_BUFFER_SIZE - 1; i++) {
		usb_buffer[i] = buffer[i];

		if(buffer[i] == 0)
			break;
	}

	usb_buffer[USB_BUFFER_SIZE -1] = 0;
	usb_ready = 1;
}

#define RELAY_DDR		DDRB
#define RELAY_PORT	PORTB
#define RELAY_PIN1	PB4
#define RELAY_PIN2	PB5
#define RELAY_PIN3	PB6
#define RELAY_PIN4	PB7

void relay_setup() {
	RELAY_DDR |= _BV(RELAY_PIN1) | _BV(RELAY_PIN2) | _BV(RELAY_PIN3) | _BV(RELAY_PIN4);
}

uint8_t relay_current=0;


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void) {
	cli();

	usb_setup();
	usb_set_callback(handle_usb);

	uart_setup();
	uart_set_callback(handle_uart);

	relay_setup();
	sei();

	char output_buffer[OUTPUT_BUFFER_SIZE];

	for (;;) {
		usb_read_loop();

		if(usb_ready) {
			output_buffer[0] = 0;

			if(strncmp(usb_buffer, "ESP ", 4) == 0) {
				sprintf(output_buffer, "> UART: %s\r\n", usb_buffer + 4);

				uart_writeln_string(usb_buffer + 4);
			}
			else if(strncmp(usb_buffer, "ON", 2) == 0) {
					RELAY_PORT |= _BV(RELAY_PIN1) | _BV(RELAY_PIN2) | _BV(RELAY_PIN3) | _BV(RELAY_PIN4);
				}
			else if(strncmp(usb_buffer, "OFF", 3) == 0) {
					RELAY_PORT &= ~(_BV(RELAY_PIN1) | _BV(RELAY_PIN2) | _BV(RELAY_PIN3) | _BV(RELAY_PIN4));
			}
			else {
				sprintf(output_buffer, "?: %s\r\n", usb_buffer);
			}

			usb_write_string(output_buffer);
			usb_ready = 0;
		}

		if(uart_ready) {
			sprintf(output_buffer, "< UART: %s\r\n", uart_buffer);
			usb_write_string(output_buffer);

			uart_ready = 0;
		}

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

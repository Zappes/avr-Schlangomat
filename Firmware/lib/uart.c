/*
 * uart.c
 *
 * Created: 15.09.2013 03:05:59
 * Author: Paul Rogalinski, paul@paul.vc
 * Modified by Gregor Ottmann, http://elektroschrott.bluephod.net
 */

#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "uart.h"

char uart_command_buffer[UART_COMMAND_BUFFER_SIZE] = {0};
char uart_output_buffer[UART_COMMAND_BUFFER_SIZE+1] = {0};

volatile uint8_t uart_ready = 0;
uint8_t uart_buffer_pos = 0;

uint8_t uart_isr_mode = UART_MODE_COMMAND;


void uart_set_isr_mode(uint8_t mode) {
	if (mode == UART_MODE_COMMAND || mode == UART_MODE_SERVER) {
		uart_isr_mode = mode;
	}
}

uint8_t uart_get_isr_mode(void) {
	return uart_isr_mode;
}

void uart_setup(void) {
	UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11); // 1 Stop-Bit, 8 Bits
	UCSR1B = (1 << TXEN1) | (1 << RXEN1); // enable RX/TX
	UCSR1B |= (1 << RXCIE1); // Enable the USART Receive Complete interrupt (USART_RXC)
			
	/* UBRRL and UBRRH � USART Baud Rate Registers */
	UBRR1H = (uint8_t) (UART_BAUD_CALC(UART_BAUD_RATE,F_CPU) >> 8);
	UBRR1L = (uint8_t) (UART_BAUD_CALC(UART_BAUD_RATE, F_CPU));

	// enable interrupts with sei() in your main program.
	// this is required for this code to work, but it's not
	// done right here in order to not interfere with your code.

	// clear command buffer.
	memset(uart_command_buffer, 0, UART_COMMAND_BUFFER_SIZE);
}

/*
 * Waits for the usart to become ready to send data and 
 * writes a single char.
 */
void uart_write_char(unsigned char c) {
	// UCSRA � USART Control and Status Register A
	// � Bit 5 � UDRE: USART Data Register Empty
	while (!(UCSR1A & (1 << UDRE1)))
		;

	UDR1 = c;
}

/*
 * Writes a string/char[] to the serial port.
 */
void uart_write_string(char *str) {
	while (*str != 0x00)
		uart_write_char(*str++);
}

void uart_write_formatted(const char* format, ...) {
	char outbuffer[UART_FORMAT_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(outbuffer, UART_FORMAT_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	uart_write_string(outbuffer);
}

void uart_writeln_string(char *str) {
	uart_write_string(str);
	uart_write_string("\r\n");
}

void uart_writeln_formatted(const char* format, ...) {
	char outbuffer[UART_FORMAT_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(outbuffer, UART_FORMAT_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	uart_write_string(outbuffer);
	uart_write_string("\r\n");
}

char* uart_get_buffer() {
	if(uart_ready) {
		uart_ready = 0;
		return uart_output_buffer;
	}

	return 0;
}

void uart_isr_handler_command(void) {
	char chr_read;
	chr_read = UDR1;

	if (chr_read != 10 && chr_read != 13) {
		uart_command_buffer[uart_buffer_pos++] = chr_read;
		uart_command_buffer[uart_buffer_pos] = 0x00;
	}

	if ((uart_buffer_pos >= (UART_COMMAND_BUFFER_SIZE - 1)) || ((chr_read == '\n' || chr_read == '\r'))) {
		if (uart_buffer_pos > 0) {
			memcpy(uart_output_buffer, uart_command_buffer, UART_COMMAND_BUFFER_SIZE);
			uart_output_buffer[UART_COMMAND_BUFFER_SIZE] = 0;
			uart_ready = 1;
		}


		// clear command buffer.
		memset(uart_command_buffer, 0, UART_COMMAND_BUFFER_SIZE);
		uart_buffer_pos = 0;
	}
}

void uart_isr_handler_server(void) {
	// todo: implement handling of server requests. these work differently - see ESP8266 documentation for +IPD
	// implement this so get/post call different handlers that have the required parameters passed to them.
}

/*
 * Handles bytes received from the serial port. Fills up the command buffer and calls
 * the bufferReadyCallback when the buffer overflows or a linefeed/newline has been
 * sent.
 */
ISR( USART1_RX_vect) {
	switch(uart_isr_mode) {
		case UART_MODE_COMMAND:
			uart_isr_handler_command();
			break;
		case UART_MODE_SERVER:
			uart_isr_handler_server();
			break;
	}
}

#endif /* USART_H_ */

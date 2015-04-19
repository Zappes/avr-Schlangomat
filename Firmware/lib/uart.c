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

char uart_command_buffer[UART_COMMAND_BUFFER_SIZE];
uint8_t uart_buffer_pos = 0;

uart_callback_t uart_buffer_ready_callback = 0x00;

/*
 * Sets the callback reference for buffer ready events 
 */
uart_callback_t uart_set_callback(uart_callback_t cb) {
	uart_callback_t old = uart_buffer_ready_callback;
	uart_buffer_ready_callback = cb;

	return old;
}

/*
 * Initializes the USART registers, enables the ISR
 */
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
	while (!(UCSR1A & (1 << UDRE1)));

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


/*
 * Handles bytes received from the serial port. Fills up the command buffer and calls
 * the bufferReadyCallback when the buffer overflows or a linefeed/newline has been
 * sent.
 */
ISR( USART1_RX_vect) {
	char chr_read;
	chr_read = UDR1;

	if(chr_read != 10 && chr_read != 13) {
		uart_command_buffer[uart_buffer_pos++] = chr_read;
		uart_command_buffer[uart_buffer_pos] = 0x00;
	}

	if ((uart_buffer_pos >= (UART_COMMAND_BUFFER_SIZE - 1))
			|| ((chr_read == '\n' || chr_read == '\r'))) {
		if (uart_buffer_ready_callback != 0x00 && uart_buffer_pos > 0)
			uart_buffer_ready_callback(uart_command_buffer);

		// clear command buffer.
		memset(uart_command_buffer, 0, UART_COMMAND_BUFFER_SIZE);
		uart_buffer_pos = 0;
	}
}

#endif /* USART_H_ */

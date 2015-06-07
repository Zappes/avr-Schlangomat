/*
 * usart.h
 *
 * Created: 15.09.2013 03:05:59
 * Author: Paul Rogalinski, paul@paul.vc
 * Modified by Gregor Ottmann, http://elektroschrott.bluephod.net
 */

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>

// @see 14.3.1 Internal Clock Generation � The Baud Rate Generator
#define UART_BAUD_RATE 9600L
#define UART_BAUD_CALC(UART_BAUD_RATE,F_CPU) ((F_CPU)/((UART_BAUD_RATE)*16l)-1)

// the size of the buffer used for formatted output
#define UART_FORMAT_BUFFER_SIZE 96

// the size of the buffer used for formatted output
#define UART_COMMAND_BUFFER_SIZE 64

#define UART_MODE_COMMAND 0
#define UART_MODE_SERVER 1

/**
 * Initializes the USART registers, enables the ISR
 */
void uart_setup(void);

/**
 * Returns the UART buffer, if it is complete, or 0 if it isn't.
 */
char* uart_get_buffer(void);

/**
 * Writes a string/char[] to the serial port.
 */
void uart_write_string(char *str);

/**
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void uart_write_formatted(const char* format, ...);

/**
 * Writes a string/char[] to the serial port.
 */
void uart_writeln_string(char *str);

/**
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void uart_writeln_formatted(const char* format, ...);

/**
 * Sets either command or server mode.
 *
 * Allowed parameters are UART_MODE_COMMAND and UART_MODE_SERVER.
 */
void uart_set_isr_mode(uint8_t mode);

/**
 * Returns the current mode.
 */
uint8_t uart_get_isr_mode(void);



#endif

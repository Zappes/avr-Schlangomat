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

typedef void (*uart_callback_t)(char* commandBuffer);

/*
 * Sets the callback reference for buffer ready events 
 */
uart_callback_t uart_set_callback(uart_callback_t cb);

/*
 * Initializes the USART registers, enables the ISR
 */
void uart_setup(void);

/*
 * Writes a string/char[] to the serial port.
 */
void uart_write_string(char *str);

/*
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void uart_write_formatted(const char* format, ...);

/*
 * Writes a string/char[] to the serial port.
 */
void uart_writeln_string(char *str);

/*
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void uart_writeln_formatted(const char* format, ...);

#endif

/*
 * usart.h
 *
 * Created: 15.09.2013 03:05:59
 * Author: Paul Rogalinski, paul@paul.vc
 * Modified by Gregor Ottmann, http://elektroschrott.bluephod.net
 */

#ifndef ESP_H_
#define ESP_H_

#include <avr/io.h>
#include <lib/usb.h>
#include <lib/debug.h>

// @see 14.3.1 Internal Clock Generation � The Baud Rate Generator
#define ESP_BAUD_RATE 9600L
#define ESP_BAUD_CALC(ESP_BAUD_RATE,F_CPU) ((F_CPU)/((ESP_BAUD_RATE)*16l)-1)

// the size of the buffer used for formatted output
#define ESP_FORMAT_BUFFER_SIZE 96

// the size of the buffer used for formatted output
#define ESP_COMMAND_BUFFER_SIZE 64

#define ESP_MODE_COMMAND 0
#define ESP_MODE_SERVER 1

#define ESP_POSITIVE_REPLY "OK"
#define ESP_NEGATIVE_REPLY "ERROR"

/**
 * Initializes the USART registers, enables the ISR
 */
void esp_setup(void);

/**
 * Executes an AT-command on the ESP chip.
 */
uint8_t esp_exec_command(char* command, char* positive_reply, char* negative_reply);

/**
 * Executes a command and writes the results to the provided buffer.
 */
uint8_t esp_exec_function(char* command, char* positive_reply, char* negative_reply, char* result_buffer, size_t result_buffer_size);

/**
 * Returns the ESP buffer, if it is complete, or 0 if it isn't.
 */
char* esp_get_buffer(void);

/**
 * Writes a string/char[] to the serial port.
 */
void esp_write_string(char *str);

/**
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void esp_write_formatted(const char* format, ...);

/**
 * Writes a string/char[] to the serial port.
 */
void esp_writeln_string(char *str);

/**
 * Writes the prompt followed by a colon, a space, the value and \r\n.
 */
void esp_writeln_formatted(const char* format, ...);

/**
 * Sets either command or server mode.
 *
 * Allowed parameters are ESP_MODE_COMMAND and ESP_MODE_SERVER.
 */
void esp_set_isr_mode(uint8_t mode);

/**
 * Returns the current mode.
 */
uint8_t esp_get_isr_mode(void);



#endif

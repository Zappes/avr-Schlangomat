/*
 * usart.h
 *
 * Created: 15.09.2013 03:05:59
 * Author: Paul Rogalinski, paul@paul.vc
 * Modified by Gregor Ottmann, http://elektroschrott.bluephod.net
 */

#ifndef ESP_H_
#define ESP_H_

#include <string.h>
#include <stdlib.h>

#include <avr/io.h>

#include "debug.h"
#include "lib/relay.h"
#include "lib/rules.h"
#include "lib/sensors.h"


// @see 14.3.1 Internal Clock Generation � The Baud Rate Generator
#define ESP_BAUD_RATE 9600L
#define ESP_BAUD_CALC(ESP_BAUD_RATE,F_CPU) ((F_CPU)/((ESP_BAUD_RATE)*16l)-1)

// the size of the buffer used for command input
#define ESP_COMMAND_BUFFER_SIZE 64

// the size of the buffer used for output formatting
#define ESP_FORMAT_BUFFER_SIZE 64

// the size of the buffer used for server request processing
#define ESP_SERVER_BUFFER_SIZE 512

#define ESP_MODE_COMMAND	0
#define ESP_MODE_SERVER		1
#define ESP_MODE_SEND			2

#define ESP_POSITIVE_REPLY "OK"
#define ESP_NEGATIVE_REPLY "ERROR"

#define ESP_NUM_CHANNELS 5

#define ESP_SEND_DELAY_MS 500

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

/**
 * Handles requests that were identified in the interrupt routine.
 *
 * Whenever the interrupt handler detects a request, it enqueues it. The main program loop is supposed
 * to call this one periodically in order to handle those requests.
 */
void esp_handle_pending_requests(void);


#endif

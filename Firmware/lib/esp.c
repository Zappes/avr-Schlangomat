#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "esp.h"

char esp_command_buffer[ESP_COMMAND_BUFFER_SIZE] = { 0 };
char esp_output_buffer[ESP_COMMAND_BUFFER_SIZE + 1] = { 0 };

volatile uint8_t esp_ready = 0;
uint8_t esp_isr_mode = ESP_MODE_COMMAND;

uint8_t request_channels[ESP_NUM_CHANNELS] = { 0 };

void esp_setup(void) {
	UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11); // 1 Stop-Bit, 8 Bits
	UCSR1B = (1 << TXEN1) | (1 << RXEN1); // enable RX/TX
	UCSR1B |= (1 << RXCIE1); // Enable the USART Receive Complete interrupt (USART_RXC)

	/* UBRRL and UBRRH � USART Baud Rate Registers */
	UBRR1H = (uint8_t) (ESP_BAUD_CALC(ESP_BAUD_RATE,F_CPU) >> 8);
	UBRR1L = (uint8_t) (ESP_BAUD_CALC(ESP_BAUD_RATE, F_CPU));

	// enable interrupts with sei() in your main program.
	// this is required for this code to work, but it's not
	// done right here in order to not interfere with your code.

	// clear command buffer.
	memset(esp_command_buffer, 0, ESP_COMMAND_BUFFER_SIZE);

	// reset the chip.
	usb_writeln_formatted("Reset: %d", esp_exec_command("AT+RST", "ready", 0));
	usb_writeln_formatted("CWMODE: %d", esp_exec_command("AT+CWMODE=1", 0, 0));
	usb_writeln_formatted("CIPMUX: %d", esp_exec_command("AT+CIPMUX=1", 0, 0));
	usb_writeln_formatted("CIPSERVER: %d", esp_exec_command("AT+CIPSERVER=1,80", 0, 0));

	esp_set_isr_mode(ESP_MODE_SERVER);
}

/**
 * Executes a command on the ESP8266 and blocks until the ESP has sent either the
 * positive or negative answer.
 *
 * It is possible to specify the expected success/error string as the fucking AT
 * firmware isn't really consistent about that. If you send AT+RST, you get OK
 * immediately. The command isn't done until you receive "ready", though ...
 *
 * Default values for pos/neg are OK and ERR. You can simply specify 0 for the
 * arguments if the defaults are OK for your command.
 */
uint8_t esp_exec_command(char* command, char* positive_reply, char* negative_reply) {
	return esp_exec_function(command, positive_reply, negative_reply, 0, 0);
}

uint8_t esp_exec_function(char* command, char* positive_reply, char* negative_reply, char* result_buffer, size_t result_buffer_size) {
	size_t result_pos = 0;
	// only use the result buffer if there's at least enough space for crlf and a null byte at the end.
	uint8_t use_buffer = (result_buffer != 0 && result_buffer_size > 2);
	uint8_t result = 0xFF;
	uint8_t old_mode = esp_get_isr_mode();

	char* esp_positive_reply = positive_reply ? positive_reply : ESP_POSITIVE_REPLY;
	char* esp_negative_reply = negative_reply ? negative_reply : ESP_NEGATIVE_REPLY;

	if (use_buffer) {
		memset(result_buffer, 0, result_buffer_size);
	}

	esp_set_isr_mode(ESP_MODE_COMMAND);

	// clear buffer
	while (esp_get_buffer())
		;

	esp_writeln_string(command);

	char* esp_result;
	uint8_t maxlines = 100;
	while (--maxlines != 0) {
		uint16_t timeout = 5000;
		while (!(esp_result = esp_get_buffer()) && --timeout != 0) {
			_delay_ms(1);
		}

		if (timeout == 0) {
			return 0xFD;
		}

		if (strncmp(esp_result, esp_positive_reply, strlen(esp_positive_reply) + 1) == 0) {
			result = 0;
			break;
		} else if (strncmp(esp_result, esp_negative_reply, strlen(esp_negative_reply) + 1) == 0) {
			result = 1;
			break;
		} else if (use_buffer) {
			// the comparison with buffer size - 3 makes sure that enough buffer is left for crlf and a null byte
			if (result_pos < result_buffer_size - 3) {
				size_t result_size = strlen(esp_result) + 2;
				size_t buffer_size = result_buffer_size - result_pos;
				size_t copy_size = result_size < buffer_size ? result_size : buffer_size;

				memcpy(result_buffer + result_pos, esp_result, copy_size);

				result_pos = result_pos + copy_size - 2;
				result_buffer[result_pos++] = '\r';
				result_buffer[result_pos++] = '\n';
			}
		}
	}

	esp_set_isr_mode(old_mode);
	return result;
}

void esp_set_isr_mode(uint8_t mode) {
	if (mode == ESP_MODE_COMMAND || mode == ESP_MODE_SERVER || mode == ESP_MODE_SEND) {
		esp_isr_mode = mode;
	}
}

uint8_t esp_get_isr_mode(void) {
	return esp_isr_mode;
}

/*
 * Waits for the usart to become ready to send data and 
 * writes a single char.
 */
void esp_write_char(unsigned char c) {
	// UCSRA � USART Control and Status Register A
	// � Bit 5 � UDRE: USART Data Register Empty
	while (!(UCSR1A & (1 << UDRE1)))
		;

	UDR1 = c;
}

/*
 * Writes a string/char[] to the serial port.
 */
void esp_write_string(char *str) {
	while (*str != 0x00)
		esp_write_char(*str++);
}

void esp_write_formatted(const char* format, ...) {
	char outbuffer[ESP_FORMAT_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(outbuffer, ESP_FORMAT_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	esp_write_string(outbuffer);
}

void esp_writeln_string(char *str) {
	esp_write_string(str);
	esp_write_string("\r\n");
}

void esp_writeln_formatted(const char* format, ...) {
	char outbuffer[ESP_FORMAT_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(outbuffer, ESP_FORMAT_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	esp_write_string(outbuffer);
	esp_write_string("\r\n");
}

char* esp_get_buffer() {
	if (esp_ready) {
		esp_ready = 0;
		return esp_output_buffer;
	}

	return 0;
}

/*
 * Handles bytes received from the serial port in command mode.
 * Fills up the command buffer and calls
 * the bufferReadyCallback when the buffer overflows or a linefeed/newline has been
 * sent.
 */
void esp_isr_handler_command(char chr_read) {
	static uint8_t buffer_pos = 0;

	if (chr_read != 10 && chr_read != 13) {
		esp_command_buffer[buffer_pos++] = chr_read;
		esp_command_buffer[buffer_pos] = 0x00;
	}

	if ((buffer_pos >= (ESP_COMMAND_BUFFER_SIZE - 1)) || ((chr_read == '\n' || chr_read == '\r'))) {
		if (buffer_pos > 0) {
			memcpy(esp_output_buffer, esp_command_buffer, ESP_COMMAND_BUFFER_SIZE);
			esp_output_buffer[ESP_COMMAND_BUFFER_SIZE] = 0;
			esp_ready = 1;
		}

		// clear command buffer.
		buffer_pos = 0;
		esp_command_buffer[0] = 0;
	}
}

void esp_isr_handler_server(char chr_read) {
#define PMODE_READ_COMMAND					0
#define PMODE_PROCESS_IPD_CHANNEL		1
#define PMODE_PROCESS_IPD_SIZE			2
#define PMODE_PROCESS_IPD_BODY			3
#define PMODE_PROCESS_IPD_STOP			4

	static char buffer[ESP_SERVER_BUFFER_SIZE + 1] = { 0 };
	static uint8_t buffer_pos = 0;
	static uint8_t parse_mode = PMODE_READ_COMMAND;
	static uint8_t channel = 0;
	static long size = 0;
	static long body_count = 0;

	switch (parse_mode) {
		case PMODE_READ_COMMAND:
			if (chr_read == 10 || chr_read == 13 || buffer_pos >= ESP_SERVER_BUFFER_SIZE) {
				buffer_pos = 0;
				buffer[0] = 0;
			} else {
				buffer[buffer_pos++] = chr_read;
				buffer[buffer_pos] = 0;

				if (strncasecmp(buffer, "+IPD,", 5) == 0) {
					parse_mode = PMODE_PROCESS_IPD_CHANNEL;
					buffer_pos = 0;
					buffer[0] = 0;
				}
			}
			break;
		case PMODE_PROCESS_IPD_CHANNEL:
			if (chr_read == ',') {
				channel = strtol(buffer, 0, 10);

				if (channel < ESP_NUM_CHANNELS) {
					parse_mode = PMODE_PROCESS_IPD_SIZE;
				} else {
					buffer_pos = 0;
					buffer[0] = 0;
					parse_mode = PMODE_READ_COMMAND;
				}
			} else {
				if (buffer_pos < 3) {
					buffer[buffer_pos++] = chr_read;
					buffer[buffer_pos] = 0;
				} else {
					buffer_pos = 0;
					buffer[0] = 0;
					parse_mode = PMODE_READ_COMMAND;
				}
			}
			break;
		case PMODE_PROCESS_IPD_SIZE:
			if (chr_read == ':') {
				size = strtol(buffer, 0, 10);

				buffer_pos = 0;
				buffer[0] = 0;
				parse_mode = PMODE_PROCESS_IPD_BODY;
			} else {
				if (buffer_pos < 5) {
					buffer[buffer_pos++] = chr_read;
					buffer[buffer_pos] = 0;
				} else {
					buffer_pos = 0;
					buffer[0] = 0;
					parse_mode = PMODE_READ_COMMAND;
				}
			}
			break;
		case PMODE_PROCESS_IPD_BODY:
			body_count++;

			// todo: read request method and do something with that information.

			if (body_count >= size) {
				buffer_pos = 0;
				buffer[0] = 0;
				parse_mode = PMODE_PROCESS_IPD_STOP;
			}
			break;
		case PMODE_PROCESS_IPD_STOP:
			if (chr_read == 10 || chr_read == 13 || buffer_pos >= ESP_SERVER_BUFFER_SIZE) {
				buffer_pos = 0;
				buffer[0] = 0;
			} else {
				buffer[buffer_pos++] = chr_read;
				buffer[buffer_pos] = 0;

				if (strncasecmp(buffer, "OK", 2) == 0) {
					buffer_pos = 0;
					buffer[0] = 0;
					parse_mode = PMODE_READ_COMMAND;
					request_channels[channel] = 1;
				}
			}
			break;
	}
}

volatile uint8_t send_status = 0;

void esp_isr_handler_send(char chr_read) {
	static char buffer[8] = { 0 };
	static uint8_t buffer_pos = 0;

	switch (send_status) {

		case 0:
			if (chr_read == '>') {
				buffer_pos = 0;
				buffer[0] = 0;
				send_status = 1;
			}
			break;
		case 1:
			if (chr_read == 10 || chr_read == 13 || buffer_pos >= 7) {
				buffer_pos = 0;
				buffer[0] = 0;
			} else {
				buffer[buffer_pos++] = chr_read;
				buffer[buffer_pos] = 0;

				if (strncasecmp(buffer, "SEND OK", 7) == 0) {
					buffer_pos = 0;
					buffer[0] = 0;
					send_status = 0;
				}
			}
		break;
	}
}

void esp_send_http_response(uint8_t channel, char* response) {
	uint8_t old_mode = esp_get_isr_mode();
	esp_set_isr_mode(ESP_MODE_SEND);

	send_status = 0;
	esp_writeln_formatted("AT+CIPSEND=%d,%d", channel, strlen(response));

	while (!send_status);
	esp_write_string(response);
	while (send_status);

	esp_set_isr_mode(old_mode);
}

void esp_handle_pending_requests(void) {
	for (uint8_t i = 0; i < ESP_NUM_CHANNELS; i++) {
		if (request_channels[i]) {
			char buf[128] = { 0 };

			Sensor_Reading s1 = sensors_read_sensor(1);
			Sensor_Reading s2 = sensors_read_sensor(2);

			char rule1[9] = {0};
			rules_print_rule(1,rule1);
			char rule2[9] = {0};
			rules_print_rule(2,rule2);
			char rule3[9] = {0};
			rules_print_rule(3,rule3);
			char rule4[9] = {0};
			rules_print_rule(4,rule4);

			sprintf(buf, "{\"sock\":[%d,%d,%d,%d],\"sens\":[{\"h\":%d.%d,\"t\":%d.%d},{\"h\":%d.%d,\"t\":%d.%d}],\"rule\":[\"%s\",\"%s\",\"%s\",\"%s\"]}", relay_state(1), relay_state(2), relay_state(3), relay_state(4), s1.humidity,
					s1.humidity_frac, s1.temperature, s1.temperature_frac, s2.humidity, s2.humidity_frac, s2.temperature, s2.temperature_frac, rule1, rule2, rule3, rule4);

			esp_send_http_response(i, buf);

			sprintf(buf, "AT+CIPCLOSE=%d", i);
			esp_exec_command(buf, 0, 0);
			request_channels[i] = 0;
		}
	}
}

ISR( USART1_RX_vect) {
	char chr_read = UDR1;

	switch (esp_isr_mode) {
		case ESP_MODE_COMMAND:
			esp_isr_handler_command(chr_read);
			break;
		case ESP_MODE_SERVER:
			esp_isr_handler_server(chr_read);
			break;
		case ESP_MODE_SEND:
			esp_isr_handler_send(chr_read);
			break;
	}
}

#endif /* USART_H_ */

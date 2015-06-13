#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "esp.h"

char esp_command_buffer[ESP_COMMAND_BUFFER_SIZE] = {0};
char esp_output_buffer[ESP_COMMAND_BUFFER_SIZE+1] = {0};

volatile uint8_t esp_ready = 0;
uint8_t esp_buffer_pos = 0;
uint8_t esp_isr_mode = ESP_MODE_COMMAND;

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
	usb_writeln_formatted("CIPMUX: %d", esp_exec_command("AT+CIPMUX=1", 0, 0));
	usb_writeln_formatted("CIPSERVER: %d", esp_exec_command("AT+CIPSERVER=1,80", 0, 0));
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

	if(use_buffer) {
		memset(result_buffer, 0, result_buffer_size);
	}

	esp_set_isr_mode(ESP_MODE_COMMAND);

	// clear buffer
	while(esp_get_buffer());

	esp_writeln_string(command);

	char* esp_result;
	uint8_t maxlines = 100;
	while (--maxlines != 0) {
		uint16_t timeout = 5000;
		while (!(esp_result = esp_get_buffer()) && --timeout != 0) {
			_delay_ms(1);
		}

		if(timeout == 0) {
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
			if(result_pos < result_buffer_size - 3) {
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
	if (mode == ESP_MODE_COMMAND || mode == ESP_MODE_SERVER) {
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
	if(esp_ready) {
		esp_ready = 0;
		return esp_output_buffer;
	}

	return 0;
}

void esp_isr_handler_command(void) {
	char chr_read;
	chr_read = UDR1;

	if (chr_read != 10 && chr_read != 13) {
		esp_command_buffer[esp_buffer_pos++] = chr_read;
		esp_command_buffer[esp_buffer_pos] = 0x00;
	}

	if ((esp_buffer_pos >= (ESP_COMMAND_BUFFER_SIZE - 1)) || ((chr_read == '\n' || chr_read == '\r'))) {
		if (esp_buffer_pos > 0) {
			memcpy(esp_output_buffer, esp_command_buffer, ESP_COMMAND_BUFFER_SIZE);
			esp_output_buffer[ESP_COMMAND_BUFFER_SIZE] = 0;
			esp_ready = 1;
		}

		// clear command buffer.
		memset(esp_command_buffer, 0, ESP_COMMAND_BUFFER_SIZE);
		esp_buffer_pos = 0;
	}
}

void esp_isr_handler_server(void) {
	// todo: implement handling of server requests. these work differently - see ESP8266 documentation for +IPD
	// implement this so get/post call different handlers that have the required parameters passed to them.
}

/*
 * Handles bytes received from the serial port. Fills up the command buffer and calls
 * the bufferReadyCallback when the buffer overflows or a linefeed/newline has been
 * sent.
 */
ISR( USART1_RX_vect) {
	switch(esp_isr_mode) {
		case ESP_MODE_COMMAND:
			esp_isr_handler_command();
			break;
		case ESP_MODE_SERVER:
			esp_isr_handler_server();
			break;
	}
}

#endif /* USART_H_ */

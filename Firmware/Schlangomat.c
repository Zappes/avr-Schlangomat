#include "Schlangomat.h"

#define OUTPUT_BUFFER_SIZE 96
#define UART_BUFFER_SIZE 64
#define USB_BUFFER_SIZE 64

volatile char uart_buffer[UART_BUFFER_SIZE] = { 0 };
volatile uint8_t uart_ready = 0;
volatile char usb_buffer[USB_BUFFER_SIZE] = { 0 };
volatile uint8_t usb_ready = 0;

void handle_uart(char* buffer) {
	for (int i = 0; i < UART_BUFFER_SIZE - 1; i++) {
		uart_buffer[i] = buffer[i];

		if (buffer[i] == 0)
			break;
	}

	uart_buffer[UART_BUFFER_SIZE - 1] = 0;
	uart_ready = 1;
}

void handle_usb(char* buffer) {
	for (int i = 0; i < USB_BUFFER_SIZE - 1; i++) {
		usb_buffer[i] = buffer[i];

		if (buffer[i] == 0)
			break;
	}

	usb_buffer[USB_BUFFER_SIZE - 1] = 0;
	usb_ready = 1;
}

void dump_sensors(void) {
	for (int i = 1; i <= SENSORS_COUNT; i++) {
		sensor_reading reading;
		int err = sensors_read_rensor(i, &reading);
		if (err) {
			usb_writeln_formatted("Sens[%d]: error %d", i, err);
		} else {
			usb_writeln_formatted("Sens[%d]: Temp %d,%dC, Hum %d,%d%%", i, reading.temperature, reading.temperature_frac, reading.humidity, reading.humidity_frac);
		}
	}
}

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

		if (usb_ready) {
			output_buffer[0] = 0;

			if (is_command(usb_buffer, "esp")) {
				sprintf(output_buffer, "> UART: %s\r\n", usb_buffer + 4);
				usb_write_string(output_buffer);
				uart_writeln_string((char*) usb_buffer + 4);
			} else if (is_command(usb_buffer, "setrule")) {
				int rule_number = atoi(usb_buffer + 7);
				char* rule_start = usb_buffer + 7;
				while (*rule_start < ':' && *rule_start != 0)
					rule_start++;
				usb_writeln_formatted("SETRULE %d [%d]:", rule_number, rules_set_rule(rule_number, rule_start));
			} else if (is_command(usb_buffer, "getrule")) {
				char buffer[8] = { 0 };
				int rule_number = atoi(usb_buffer + 7);
				int result = rules_print_rule(rule_number, buffer);
				usb_writeln_formatted("GETRULE %d [%d]: %s", rule_number, result, buffer);
			} else if (is_command(usb_buffer, "on")) {
				relay_on(get_num_from_param((char*) usb_buffer + 2, 4));
			} else if (is_command(usb_buffer, "off")) {
				relay_off(get_num_from_param((char*) usb_buffer + 3, 4));
			} else if (is_command(usb_buffer, "state")) {
				int relay_num = get_num_from_param((char*) usb_buffer + 5, 4);

				if (relay_num) {
					usb_writeln_formatted("Pig[%d]: %s", relay_num, relay_state(relay_num) ? "on" : "off");
				} else {
					for (int i = 1; i <= RELAY_COUNT; i++) {
						usb_writeln_formatted("Pig[%d]: %s", i, relay_state(i) ? "on" : "off");
					}
				}
			} else if (is_command(usb_buffer, "sens")) {
				dump_sensors();
			} else {
				usb_writeln_formatted("?: %s\r\n", usb_buffer);
			}

			usb_ready = 0;
		}

		if (uart_ready) {
			sprintf(output_buffer, "< UART: %s\r\n", uart_buffer);
			usb_write_string(output_buffer);

			uart_ready = 0;
		}

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

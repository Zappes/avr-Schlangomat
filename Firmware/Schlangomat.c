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
	for (uint8_t i = 1; i <= SENSORS_COUNT; i++) {
		Sensor_Reading reading = sensors_read_sensor(i);
		if (reading.error) {
			usb_writeln_formatted("Sens[%d]: error %d x%d", i, reading.error, reading.error_count);
		} else {
			usb_writeln_formatted("Sens[%d]: Temp %d,%dC, Hum %d,%d%%", i, reading.temperature, reading.temperature_frac, reading.humidity, reading.humidity_frac);
		}
	}
}

void print_rule(uint8_t rule_number) {
	char buffer[10] = { 0 };

	int result = rules_print_rule(rule_number, buffer);
	usb_write_formatted("Rule[%d]: ", rule_number);

	switch(result) {
		case 0:
			usb_writeln_string(buffer);
			break;
		case RULES_ERR_INACTIVE:
			usb_writeln_string("inactive");
			break;
		case RULES_ERR_INDEX:
			usb_writeln_string("index error");
			break;
		default:
			usb_writeln_string("unknown error");
	}
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void) {
	cli();

	usb_setup(handle_usb);
	uart_setup(handle_uart);
	relay_setup();
	rules_setup();
	sensors_setup();
	timer_setup(SENSOR_INTERVAL_SECS);

	sei();

	char output_buffer[OUTPUT_BUFFER_SIZE];

	for (;;) {
		if(timer_ready()) {
			sensors_update_sensor(0);
			rules_execute();
			timer_done();
		}

		usb_read_loop();
		if (usb_ready) {
			output_buffer[0] = 0;
			char* input = (char*)usb_buffer;

			if (is_command(input, "esp")) {
				uart_writeln_string(input + 4);
			} else if (is_command(input, "setrule")) {
				int rule_number = get_num_from_param(input + 7, RULES_COUNT);
				char* rule_start = input + 7;
				while (*rule_start < ':' && *rule_start != 0)
					rule_start++;
				rules_set_rule(rule_number, rule_start);
			} else if (is_command(input, "getrule")) {
				int rule_number = get_num_from_param(input + 7, RULES_COUNT);

				if(rule_number == 0) {
					for(int i = 1; i <= RULES_COUNT; i++) {
						print_rule(i);
					}
				}
				else {
					print_rule(rule_number);
				}
			} else if (is_command(input, "on")) {
				relay_on(get_num_from_param(input + 2, 4));
			} else if (is_command(input, "off")) {
				relay_off(get_num_from_param(input + 3, 4));
			} else if (is_command(input, "state")) {
				int relay_num = get_num_from_param(input + 5, 4);

				if (relay_num) {
					usb_writeln_formatted("Pig[%d]: %s", relay_num, relay_state(relay_num) ? "on" : "off");
				} else {
					for (int i = 1; i <= RELAY_COUNT; i++) {
						usb_writeln_formatted("Pig[%d]: %s", i, relay_state(i) ? "on" : "off");
					}
				}
			} else if (is_command(input, "sens")) {
				dump_sensors();
			} else if (is_command(input, "setinterval")) {
				timer_set_scale_limit(get_num_from_param(input + 11, 255));
			} else if (is_command(input, "getinterval")) {
				usb_writeln_formatted("Interval: %d", timer_get_scale_limit());
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

#include "Schlangomat.h"

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

	switch (result) {
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

	usb_setup();
	sensors_setup();
	relay_setup();
	rules_setup();
	timer_setup(SENSOR_INTERVAL_SECS);

	sei();

	// setting up the esp needs functioning communications, so we have to do it AFTER interrupts
	// are enabled.
	esp_setup();

	char* input;
	char output[128] = {0};

	for (;;) {
		if (timer_ready()) {
			sensors_update_sensor(0);
			rules_execute();
			timer_done();
		}

		input = usb_get_buffer();
		if (input) {
			if (is_command(input, "espsetup")) {
				esp_setup();
			} else if (is_command(input, "esp")) {
				uint8_t result = esp_exec_function(input + 4, 0, 0, output, sizeof(output));
				usb_writeln_string(output);
				usb_writeln_formatted("ESP: %d", result);
			} else if (is_command(input, "setrule")) {
				int rule_number = get_num_from_param(input + 7, RULES_COUNT);
				char* rule_start = input + 7;
				while (*rule_start < ':' && *rule_start != 0)
					rule_start++;
				rules_set_rule(rule_number, rule_start);
			} else if (is_command(input, "getrule")) {
				int rule_number = get_num_from_param(input + 7, RULES_COUNT);

				if (rule_number == 0) {
					for (int i = 1; i <= RULES_COUNT; i++) {
						print_rule(i);
					}
				} else {
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
				usb_writeln_formatted("?: %s\r\n", input);
			}
		}

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
		esp_handle_pending_requests();
	}
}

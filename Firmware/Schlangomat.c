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

//============================================================================================
// ESP stuff
//============================================================================================
#define ESP_POSITIVE_REPLY "OK"
#define ESP_NEGATIVE_REPLY "ERROR"

volatile uint8_t esp_waitresult = 0xFF;
char* esp_positive_reply = ESP_POSITIVE_REPLY;
char* esp_negative_reply = ESP_NEGATIVE_REPLY;

void esp_handle_http(char* buffer) {
	static int status[4] = 0;


	if(is_command(buffer+2, "connect")) {

		status = 1;
	}
	else if(status == 1 && is_command(buffer, "ok")) {
		status = 0;
	}
}

void esp_handle_result(char* buffer) {
	if(strncmp(buffer, esp_positive_reply, strlen(esp_positive_reply) + 1) == 0) {
		esp_waitresult = 0;
	}
	else	if(strncmp(buffer, esp_negative_reply, strlen(esp_negative_reply) + 1) == 0) {
		esp_waitresult = 1;
	}
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
	esp_waitresult = 0xFF;

	// set the markers for negative/positive replies
	if(positive_reply != 0) {
		esp_positive_reply = positive_reply;
	}

	if(negative_reply != 0) {
		esp_negative_reply = negative_reply;
	}

	uart_callback_t cb = uart_set_callback(esp_handle_result);

	uart_writeln_string(command);

	// as long as the result code hasn't been changed by the handler, wait.
	// if there's no result after the timeout, stop waiting.
	int counter = 500;
	while((esp_waitresult == 0xFF) && (--counter != 0)) {
		_delay_ms(10);
	}

	uart_set_callback(cb);

	esp_positive_reply = ESP_POSITIVE_REPLY;
	esp_negative_reply = ESP_NEGATIVE_REPLY;

	return esp_waitresult;
}

void esp_setup(void) {
	// reset the chip.
	usb_writeln_formatted("Reset: %d", esp_exec_command("AT+RST", "ready", 0));
	usb_writeln_formatted("CIPMUX: %d", esp_exec_command("AT+CIPMUX=1", 0, 0));
	usb_writeln_formatted("CIPSERVER: %d", esp_exec_command("AT+CIPSERVER=1,80", 0, 0));
}
//============================================================================================




/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void) {
	cli();

	usb_setup(handle_usb);
	sensors_setup();
	relay_setup();
	rules_setup();
	timer_setup(SENSOR_INTERVAL_SECS);
	uart_setup(handle_uart);

	sei();

	// setting up the esp needs functioning communications, so we have to do it AFTER interrupts
	// are enabled.
	esp_setup();

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

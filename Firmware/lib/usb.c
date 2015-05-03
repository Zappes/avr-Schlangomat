/*
 * usb.c
 *
 *  Created on: 16.04.2015
 *      Author: zaphod
 */

#include "usb.h"

volatile char usb_input_buffer[USB_INPUT_BUFFER_SIZE] = { 0 };
uint8_t usb_command_pos = 0;

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = { .Config = { .ControlInterfaceNumber = INTERFACE_ID_CDC_CCI, .DataINEndpoint =
		{ .Address = CDC_TX_EPADDR, .Size = CDC_TXRX_EPSIZE, .Banks = 1, }, .DataOUTEndpoint = { .Address = CDC_RX_EPADDR, .Size = CDC_TXRX_EPSIZE, .Banks = 1, }, .NotificationEndpoint = { .Address =
CDC_NOTIFICATION_EPADDR, .Size =
CDC_NOTIFICATION_EPSIZE, .Banks = 1, }, }, };

usb_callback_t usb_buffer_ready_callback = 0x00;

/*
 * Sets the callback reference for buffer ready events
 */
usb_callback_t usb_set_callback(usb_callback_t cb) {
	usb_callback_t old = usb_buffer_ready_callback;
	usb_buffer_ready_callback = cb;

	return old;
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void usb_setup(void) {
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
}

void usb_read_loop() {
	int currentValue;

	do {
		currentValue = fgetc(&USBSerialStream);

		if (currentValue != EOF) {
			if (usb_command_pos >= (USB_INPUT_BUFFER_SIZE - 1) || currentValue == 10 || currentValue == 13) {

				usb_buffer_ready_callback(usb_input_buffer);

				usb_command_pos = 0;
				usb_input_buffer[0] = 0;
			} else {
				usb_input_buffer[usb_command_pos++] = (char) currentValue;
				usb_input_buffer[usb_command_pos] = 0;
			}
		}
	} while (currentValue != EOF);
}

/*
 * Writes a string/char[] to the serial port.
 */
void usb_write_string(char *str) {
	fputs(str, &USBSerialStream);
}

void usb_write_formatted(const char* format, ...) {
	char outbuffer[USB_FORMAT_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(outbuffer, USB_FORMAT_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	usb_write_string(outbuffer);
}

void usb_writeln_string(char *str) {
	usb_write_string(str);
	usb_write_string("\r\n");
}

void usb_writeln_formatted(const char* format, ...) {
	char outbuffer[USB_FORMAT_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(outbuffer, USB_FORMAT_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	usb_write_string(outbuffer);
	usb_write_string("\r\n");
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) {
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void) {
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void) {
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);

	// discard command buffer - this fixes a problem where modem-manager in linux might send
	// 10 bytes of useless data while probing the device.
	usb_command_pos = 0;
	usb_input_buffer[0] = 0;
}


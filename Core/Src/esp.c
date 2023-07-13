/*
 * esp.c
 *
 *  Created on: Jul 12, 2023
 *      Author: dominik
 */

#include "esp.h"
#include "uart.h"
#include "can_tx.h"

uint8_t mode = UART_MODE_RESET;
uint8_t reset_ix = 0;

uint8_t command = 0x00;
uint8_t size_buf[2];
uint8_t size_ix = 0;
uint16_t size = 0;
uint8_t text_buf[128];
uint8_t text_ix = 0;

uint8_t msgCnt = 0;

void _esp_parse_command();

void esp_reset() {
	mode = UART_MODE_RESET;
}

void esp_receive_uart() {
	int available = uart_data_available();
	if (available) {
		uint8_t txdata[2] = { available & 0xFF, msgCnt };
		can_tx_send_packet(0x001, txdata, 2);

		msgCnt++;

		uint8_t txdata2[1] = { 0x69 };
		can_tx_send_packet(0x002, txdata2, 1);

		for (int i = 0; i < available; i++) {
			uart_get_byte();
		}
	}
}

void stuff() {
	/*uint16_t readIx = 0;
		while (readIx < Size) {
			if (mode == UART_MODE_RESET) {
				if (reset_ix == 0) {
					if (uart_byte_buf[readIx++] == 0xAA)
						reset_ix++;
				} else if (reset_ix == 1) {
					if (uart_byte_buf[readIx++] == 0xFF)
						reset_ix++;
					else
						reset_ix = 0;
				} else if (reset_ix == 2) {
					if (uart_byte_buf[readIx++] == 0x55)
						reset_ix++;
					else
						reset_ix = 0;
				} else if (reset_ix == 3) {
					if (uart_byte_buf[readIx++] == 0x00)
						reset_ix++;
					else
						reset_ix = 0;
				}

				if (reset_ix == 4) {
					mode = UART_MODE_WAIT;
					command = UART_MODE_WAIT;
				}
			} else if (command == 0x00) {
				command = uart_byte_buf[readIx++];
				if (command > 0x80 && command < 0x90) {
					mode = UART_MODE_READ_SIZE;
					size_ix = 0;
				} else {
					command = 0x00;
				}
			} else {
				if (mode == UART_MODE_READ_SIZE) {
					size_buf[size_ix++] = uart_byte_buf[readIx++];

					if (size_ix == 2) {
						size = size_buf[1] << 8 | size_buf[0];
						mode = UART_MODE_READ_TEXT;
						text_ix = 0;
					}
				} else if (mode == UART_MODE_READ_TEXT) {
					if (text_ix < 128) {
						text_buf[text_ix] = uart_byte_buf[readIx++];
					}

					text_ix++;

					if (text_ix >= size) {
						_uart_parse_command();
					}
				}
			}
		}*/
}


void _esp_parse_command() {
	if (size > 128)
		size = 128;

	switch (command) {
	case CMD_TITLE:
		can_tx_set_title(text_buf, size);
		break;
	case CMD_ARTIST:
		can_tx_set_artist(text_buf, size);
		break;
	case CMD_ALBUM:
		can_tx_set_album(text_buf, size);
		break;
	default:
		break;
	}

	mode = UART_MODE_WAIT;
	command = UART_MODE_WAIT;
}

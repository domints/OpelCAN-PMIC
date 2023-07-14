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
void _esp_process_uart_byte(uint8_t data);

void esp_reset() {
	mode = UART_MODE_RESET;
}

void esp_receive_uart() {
	int available = uart_data_available();
	if (available) {
		uint8_t txdata2[5] = { mode, command, size >> 8 & 0xFF, size & 0xFF, text_ix };
		can_tx_send_packet(0x001, txdata2, 5);

		for (int i = 0; i < available; i++) {
			uint8_t value = uart_get_byte() & 0xFF;
			_esp_process_uart_byte(value);
		}
	}
}

void _esp_process_uart_byte(uint8_t value) {
	if (mode == UART_MODE_RESET) {
		if (reset_ix == 0) {
			if (value == 0xAA)
				reset_ix++;
		} else if (reset_ix == 1) {
			if (value == 0xFF)
				reset_ix++;
			else
				reset_ix = 0;
		} else if (reset_ix == 2) {
			if (value == 0x55)
				reset_ix++;
			else
				reset_ix = 0;
		} else if (reset_ix == 3) {
			if (value == 0x00)
				reset_ix++;
			else
				reset_ix = 0;
		}

		if (reset_ix == 4) {
			mode = UART_MODE_WAIT;
			command = UART_MODE_WAIT;
		}
	} else if (command == 0x00) {
		command = value;
		if (command > 0x80 && command < 0x90) {
			mode = UART_MODE_READ_SIZE;
			size_ix = 0;
		} else {
			command = 0x00;
		}
	} else {
		if (mode == UART_MODE_READ_SIZE) {
			size_buf[size_ix++] = value;

			if (size_ix == 2) {
				size = size_buf[1] << 8 | size_buf[0];
				mode = UART_MODE_READ_TEXT;
				text_ix = 0;
			}
		} else if (mode == UART_MODE_READ_TEXT) {
			if (text_ix < 128) {
				text_buf[text_ix] = value;
			}

			text_ix++;

			if (text_ix >= size) {
				_esp_parse_command();
			}
		}
	}
}

uint16_t title_sz;
uint16_t album_sz;
uint16_t artist_sz;
uint8_t debug_ctr;
void _esp_debug_size() {
	uint8_t txdata2[7] = {
			debug_ctr++,
			title_sz >> 8 & 0xFF, title_sz & 0xff,
			album_sz >> 8 & 0xFF, album_sz & 0xff,
			artist_sz >> 8 & 0xFF, artist_sz & 0xff
	};
	can_tx_send_packet(0x002, txdata2, 7);
}

void _esp_parse_command() {
	if (size > 128)
		size = 128;

	switch (command) {
	case CMD_TITLE:
		_esp_debug_size();
		can_tx_set_title(text_buf, size);
		break;
	case CMD_ARTIST:
		_esp_debug_size();
		can_tx_set_artist(text_buf, size);
		break;
	case CMD_ALBUM:
		_esp_debug_size();
		can_tx_set_album(text_buf, size);
		break;
	default:
		break;
	}

	mode = UART_MODE_WAIT;
	command = UART_MODE_WAIT;
}

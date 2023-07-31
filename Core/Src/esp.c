/*
 * esp.c
 *
 *  Created on: Jul 12, 2023
 *      Author: dominik
 */

#include "esp.h"
#include "main.h"
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

volatile bool next_song_requested = false;
volatile bool prev_song_requested = false;
volatile bool disconnect_requested = false;
volatile bool reconnect_requested = false;
volatile bool connected = false;
volatile bool push_metadata = false;

void _esp_parse_command();
void _esp_process_uart_byte(uint8_t data);

void esp_reset() {
	mode = UART_MODE_RESET;
}

bool started = false;
bool shutdown_requested = false;
uint8_t shutdown_ticks = 0;

void esp_start() {
	if (!started) {
		HAL_GPIO_WritePin(BUCK_EN_GPIO_Port, BUCK_EN_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(CAN_LP_GPIO_Port, CAN_LP_Pin, GPIO_PIN_RESET);
		connected = true;
		started = true;
	}
}

void esp_kill() {
	if (started) {
		shutdown_ticks = 0;
		shutdown_requested = true;
		started = false;
		esp_reset();
	}
}

void esp_tick() {
	if (shutdown_requested) {
		if (shutdown_ticks == 0) {
			uint8_t data[1] = { CMD_SHUTDOWN };
			uart_send_data(data, 1);
		}
		if (shutdown_ticks >= 4) {
			HAL_GPIO_WritePin(CAN_LP_GPIO_Port, CAN_LP_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(BUCK_EN_GPIO_Port, BUCK_EN_Pin, GPIO_PIN_RESET);
			shutdown_requested = false;
		}
		else
		{
			shutdown_ticks++;
		}
	}
}

void esp_receive_uart() {
	if (!started) {
		return;
	}

	int available = uart_data_available();
	if (available) {
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

void esp_next_song() {
	next_song_requested = true;
}

void esp_prev_song() {
	prev_song_requested = true;
}

void esp_disconnect() {
	disconnect_requested = true;
}

void esp_reconnect() {
	reconnect_requested = true;
}

bool esp_is_connected() {
	return connected;
}

void esp_run_can_events() {
	if (prev_song_requested) {
		prev_song_requested = false;
		uint8_t data[1] = { CMD_PREV };
		uart_send_data(data, 1);
	}

	if (next_song_requested) {
		next_song_requested = false;
		uint8_t data[1] = { CMD_NEXT };
		uart_send_data(data, 1);
	}

	if (disconnect_requested) {
		disconnect_requested = false;
		//uint8_t data[1] = { CMD_SHUTDOWN };
		//uart_send_data(data, 1);
		connected = false;
	}

	if (reconnect_requested) {
		reconnect_requested = false;
		//uint8_t data[1] = { CMD_START };
		//uart_send_data(data, 1);
		connected = true;
	}

	if (push_metadata) {
		push_metadata = false;
		can_tx_send_music_metadata();
	}
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
	case CMD_TIME:
		push_metadata = true;
	default:
		break;
	}

	mode = UART_MODE_WAIT;
	command = UART_MODE_WAIT;
}

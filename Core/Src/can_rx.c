/*
 * can_rx.h
 *
 *  Created on: Jul 17, 2023
 *      Author: dominik
 */

#include "can_rx.h"
#include "can_tx.h"
#include "esp.h"
#include "main.h"

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint8_t display_packets = 0;
uint8_t seen_packets = 0;

uint32_t last_seen = 0;

bool _can_rx_is_aux_packet(uint8_t *data);
bool _can_rx_is_non_aux_packet(uint8_t *data);

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hCan) {
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(hCan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
		/* Reception Error */
		Error_Handler();
	}

	if ((RxHeader.StdId == OPEL_POWER_ID) && (RxHeader.IDE == CAN_ID_STD)
			&& (RxHeader.DLC == 8)) {
		if (RxData[6] & 0x40) {
			esp_start();
		} else {
			esp_kill();
		}

		return;
	}
	else if ((RxHeader.StdId == DISPLAY_CAN_ID) && (RxHeader.IDE == CAN_ID_STD) && (RxHeader.DLC == 8)) {
		if (RxData[0] == 0x10) {
			if (_can_rx_is_aux_packet(RxData)) {
				can_tx_corrupt_ehu_packet();
				if (!esp_is_connected()) {
					esp_reconnect();
				}
			}
			else if (_can_rx_is_non_aux_packet(RxData)) {
				if (esp_is_connected()) {
					esp_disconnect();
				}
			}

			uint16_t len = ((RxData[0] & 0x0F) << 8 | RxData[1]) + 1;
			display_packets = len / 7;
			if (len % 7 > 0)
				display_packets += 1;

			seen_packets = 1;

			last_seen = 0;
		}
		else {
			seen_packets++;

			if (
#ifdef ONLY_AUX
					esp_is_connected() &&
#endif
					seen_packets == display_packets)
				can_tx_send_music_metadata();
		}
	}
	else if (RxHeader.StdId == STEERING_WHEEL && RxHeader.IDE == CAN_ID_STD && RxHeader.DLC == 3 && RxData[0] == 0x00) {
		if (RxData[1] == BTN_RIGHT_UP)
			esp_next_song();
		else if (RxData[1] == BTN_RIGHT_DOWN)
			esp_prev_song();
	}
}

void can_rx_tick() {
	last_seen++;
}

uint32_t can_rx_last_seen() {
	return last_seen;
}

bool _can_rx_is_aux_packet(uint8_t *data) {
	return
			(data[1] == 0x2E && data[2] == 0xC0 && data[5] == 0x03 && data[6] == 0x01 && data[7] == 0x01) ||
			(data[1] == 0x36 && data[2] == 0xC0 && data[5] == 0x03 && data[6] == 0x01 && data[7] == 0x05) ||
			(data[2] == 0xC0 && data[5] == 0x03 && data[6] == 0x01 && data[7] == 0x03) ||
			(data[2] == 0x40 && data[5] == 0x03 && data[6] == 0x01 && data[7] == 0x03);
}

bool _can_rx_is_non_aux_packet(uint8_t *data) {
	return (data[1] != 0x2E && data[2] == 0xC0 && data[5] == 0x03 && data[6] == 0x01 && data[7] == 0x01) ||
		   (data[2] == 0x40 && data[5] == 0x03 && data[6] == 0x01 && data[7] == 0x01);
}

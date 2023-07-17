/*
 * can_tx.c
 *
 *  Created on: Jul 11, 2023
 *      Author: dominik
 */
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "can_tx.h"

CAN_HandleTypeDef can_tx_hcan;

CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
uint32_t TxMailbox;

CAN_Tx_Msg_t can_tx_mailbox[CAN_MAILBOX_LENGTH];
bool can_tx_cts = true;
bool corrupt_ehu = false;
uint8_t can_tx_head = 0;
uint8_t can_tx_tail = 0;

uint16_t ticks_since_last_corrupt = 0;


uint8_t audio_title[128];
uint8_t audio_title_len;
uint8_t audio_artist[128];
uint8_t audio_artist_len;
uint8_t audio_album[128];
uint8_t audio_album_len;

void _can_tx_send_msg(CAN_Tx_Msg_t *message);
void _can_tx_send_and_dequeue();
void _can_tx_try_send_metadata();

void can_tx_set_hcan(CAN_HandleTypeDef *hcan) {
	can_tx_hcan = *hcan;

	TxHeader.ExtId = 0x01;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.TransmitGlobalTime = DISABLE;
}

void can_tx_set_title(uint8_t *buf, uint8_t len) {
	memcpy(audio_title, buf, len);
	audio_title_len = len;
	_can_tx_try_send_metadata();
}

void can_tx_set_artist(uint8_t *buf, uint8_t len) {
	memcpy(audio_artist, buf, len);
	audio_artist_len = len;
	_can_tx_try_send_metadata();
}

void can_tx_set_album(uint8_t *buf, uint8_t len) {
	memcpy(audio_album, buf, len);
	audio_album_len = len;
	_can_tx_try_send_metadata();
}

void _can_tx_try_send_metadata() {
	if (ticks_since_last_corrupt < CAN_SEND_BEFORE) {
		can_tx_send_music_metadata();
	}
}

void can_tx_send_packet(uint32_t id, uint8_t *data, uint8_t len) {
	if (can_tx_head == can_tx_tail && can_tx_cts) {
		CAN_Tx_Msg_t msg = { .id = id, .len = len };

		memcpy(msg.data, data, len);

		_can_tx_send_msg(&msg);
		return;
	}

	uint8_t next;

	next = can_tx_head + 1; // next is where head will point to after this write.
	if (next >= CAN_MAILBOX_LENGTH)
		next = 0;

	if (next == can_tx_tail) // if the head + 1 == tail, circular buffer is full
		return;

	can_tx_mailbox[can_tx_head].id = id;
	can_tx_mailbox[can_tx_head].len = len;
	memcpy(can_tx_mailbox[can_tx_head].data, data, len);

	can_tx_head = next;

	if (can_tx_cts) {
		_can_tx_send_and_dequeue();
	}
}

void can_tx_corrupt_ehu_packet() {
	ticks_since_last_corrupt = 0;
	corrupt_ehu = true;
}

void can_tx_send_next() {
	ticks_since_last_corrupt++;
	if (corrupt_ehu) {
		uint8_t data[8] = { 0x10,0x2E,0xC0,0x00,0x2B,0x03,0x01,0x01 };
		can_tx_send_packet(0x6C1, data, 8);
		corrupt_ehu = false;
	}
	if (can_tx_cts) {
		_can_tx_send_and_dequeue();
	}
}

void _can_tx_send_and_dequeue() {
	int next;

	if (can_tx_head == can_tx_tail) // if the head == tail, we don't have any data
		return;

	next = can_tx_tail + 1; // next is where tail will point to after this read.
	if (next >= CAN_MAILBOX_LENGTH)
		next = 0;

	_can_tx_send_msg(&can_tx_mailbox[can_tx_tail]);  // Read data and then move
	can_tx_tail = next;
}

void _can_tx_send_msg(CAN_Tx_Msg_t *message) {
	can_tx_cts = false;
	TxHeader.StdId = message->id;
	TxHeader.DLC = message->len;

	memcpy(TxData, message->data, message->len);

	if (HAL_CAN_AddTxMessage(&can_tx_hcan, &TxHeader, TxData, &TxMailbox)
			!= HAL_OK) {
		Error_Handler();
	}
}

uint8_t _next_tp_id(uint8_t lastId) {
	if (lastId == 0x10) {
		return 0x21;
	}
	if (lastId == 0x2F) {
		return 0x20;
	}
	return lastId + 1;
}

void can_tx_send_music_metadata() {
	uint8_t space_data[2] = { 0x00, 0x20 };
	uint8_t title_len = audio_title_len;
	uint8_t artist_len = audio_artist_len;
	uint8_t album_len = audio_album_len;
	uint8_t * title = audio_title;
	uint8_t * artist = audio_artist;
	uint8_t * album = audio_album;
	if (title_len == 0) {
		title_len = 2;
		title = space_data;
	}

	if (artist_len == 0) {
		artist_len = 2;
		artist = space_data;
	}

	if (album_len == 0) {
		album_len = 2;
		album = space_data;
	}

	uint8_t tpSize = 30 + title_len + audio_artist_len + audio_album_len;

	uint8_t buffer[8] = { 0 };
	uint8_t msgIx = 0x10;
	uint8_t bfrIx = 0;

	uint8_t buf_1[8] =
			{ msgIx, tpSize, 0x40, 0x00, tpSize - 3, 0x03, 0x01, 0x05 };
	uint8_t buf_2[8] = { msgIx = _next_tp_id(msgIx), 0x00, 0x41, 0x00, 0x75,
			0x00, 0x63, 0x00 };
	uint8_t buf_3[8] = { msgIx = _next_tp_id(msgIx), 0x69, 0x00, 0x6F, 0x02,
			0x03, 0x00, 0x41 };
	uint8_t buf_4[8] = { msgIx = _next_tp_id(msgIx), 0x00, 0x75, 0x00, 0x78,
			0x10, title_len / 2, title[0] };
	can_tx_send_packet(DISPLAY_CAN_ID, buf_1, 8);
	can_tx_send_packet(DISPLAY_CAN_ID, buf_2, 8);
	can_tx_send_packet(DISPLAY_CAN_ID, buf_3, 8);
	can_tx_send_packet(DISPLAY_CAN_ID, buf_4, 8);
	buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
	for (uint8_t i = 1; i < title_len; i++) {
		buffer[bfrIx++] = title[i];
		if (bfrIx == 8) {
			can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
			bfrIx = 0;
			buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
		}
	}

	buffer[bfrIx++] = 0x11;
	if (bfrIx == 8) {
		can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
		bfrIx = 0;
		buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
	}

	buffer[bfrIx++] = artist_len / 2;
	if (bfrIx == 8) {
		can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
		bfrIx = 0;
		buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
	}

	for (uint8_t i = 0; i < artist_len; i++) {
		buffer[bfrIx++] = artist[i];
		if (bfrIx == 8) {
			can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
			bfrIx = 0;
			buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
		}
	}

	buffer[bfrIx++] = 0x12;
	if (bfrIx == 8) {
		can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
		bfrIx = 0;
		buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
	}

	buffer[bfrIx++] = album_len / 2;
	if (bfrIx == 8) {
		can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
		bfrIx = 0;
		buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
	}

	for (uint8_t i = 0; i < album_len; i++) {
		buffer[bfrIx++] = album[i];
		if (bfrIx == 8) {
			can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
			bfrIx = 0;
			buffer[bfrIx++] = msgIx = _next_tp_id(msgIx);
		}
	}

	while (bfrIx < 8) {
		buffer[bfrIx++] = 0x00;
	}

	can_tx_send_packet(DISPLAY_CAN_ID, buffer, 8);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
	can_tx_cts = true;
}

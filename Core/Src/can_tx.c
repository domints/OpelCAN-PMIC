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
uint8_t can_tx_head = 0;
uint8_t can_tx_tail = 0;

uint8_t audio_title[128];
uint8_t audio_title_len;
uint8_t audio_artist[128];
uint8_t audio_artist_len;
uint8_t audio_album[128];
uint8_t audio_album_len;

void _can_tx_send_msg(CAN_Tx_Msg_t *message);
void _can_tx_send_and_dequeue();

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
}

void can_tx_set_artist(uint8_t *buf, uint8_t len) {
	memcpy(audio_artist, buf, len);

	audio_artist_len = len;
}

void can_tx_set_album(uint8_t *buf, uint8_t len) {
	memcpy(audio_album, buf, len);

	audio_album_len = len;
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

void _can_tx_send_and_dequeue() {
	int next;

	if (can_tx_head == can_tx_tail)  // if the head == tail, we don't have any data
		return;

	next = can_tx_tail + 1;  // next is where tail will point to after this read.
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



void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
	can_tx_cts = true;
	_can_tx_send_and_dequeue();
}

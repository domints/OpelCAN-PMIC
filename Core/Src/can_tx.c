/*
 * can_tx.c
 *
 *  Created on: Jul 11, 2023
 *      Author: dominik
 */

#include "can_tx.h"

uint8_t audio_title[128];
uint8_t audio_title_len;
uint8_t audio_artist[128];
uint8_t audio_artist_len;
uint8_t audio_album[128];
uint8_t audio_album_len;

void can_tx_set_title(uint8_t *buf, uint8_t len) {
	for (uint8_t ix = 0; ix < len; ix++) {
		audio_title[ix] = buf[ix];
	}

	audio_title_len = len;
}

void can_tx_set_artist(uint8_t *buf, uint8_t len) {
	for (uint8_t ix = 0; ix < len; ix++) {
		audio_artist[ix] = buf[ix];
	}

	audio_artist_len = len;
}

void can_tx_set_album(uint8_t *buf, uint8_t len) {
	for (uint8_t ix = 0; ix < len; ix++) {
		audio_album[ix] = buf[ix];
	}

	audio_album_len = len;
}


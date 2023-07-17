/*
 * circ_buf.c
 *
 *  Created on: Jul 12, 2023
 *      Author: dominik
 */

#include "circ_buf.h"
#include <string.h>

int circ_buf_push(circ_buf_t *c, uint8_t data) {
	int next;

	next = c->head + 1;  // next is where head will point to after this write.
	if (next >= c->maxlen)
		next = 0;

	if (next == c->tail)  // if the head + 1 == tail, circular buffer is full
		return -1;

	c->buffer[c->head] = data;  // Load data and then move
	c->head = next;             // head to next data offset.
	return 0;  // return success to indicate successful push.
}

int circ_buf_push_bytes(circ_buf_t *c, uint8_t *data, int size) {
	if (size > c->maxlen) {
		return -1;
	}

	int sizeToWrite = size;

	int maxSize = 0;
	if (c->tail > c-> head) {
		maxSize = c->tail - c->head - 1;
	}
	else {
		maxSize = c->maxlen - c->head + c->tail - 1;
	}

	if (maxSize < sizeToWrite)
		sizeToWrite = maxSize;

	int firstBatch = 0;
	int secondBatch = 0;

	if (c->head + sizeToWrite > c->maxlen) {
		firstBatch = c->maxlen - c->head;
		secondBatch = sizeToWrite - firstBatch;
	} else {
		firstBatch = sizeToWrite;
	}

	int data_ix = 0;
	memcpy(c->buffer + c->head, data, firstBatch);
	c->head += firstBatch;
	data_ix += firstBatch;

	if (c->head >= c->maxlen)
		c->head = 0;

	if (secondBatch > 0) {
		memcpy(c->buffer, data + data_ix, secondBatch);
		c->head += secondBatch;
	}

	return sizeToWrite;
}

int circ_buf_pop(circ_buf_t *c) {
	int next;

	if (c->head == c->tail)  // if the head == tail, we don't have any data
		return -1;

	next = c->tail + 1;  // next is where tail will point to after this read.
	if (next >= c->maxlen)
		next = 0;

	uint8_t val = c->buffer[c->tail];  // Read data and then move
	c->tail = next;              // tail to next offset.
	return val;  // return success to indicate successful push.
}

int circ_buf_pop_bytes(circ_buf_t *c, uint8_t *data, int size) {
	if (size > c->maxlen || c->head == c->tail) {
			return -1;
	}

	int firstBatch = 0;
	int secondBatch = 0;

	if (c->head > c->tail) {
		firstBatch = c->head - c->tail;
	}
	else {
		firstBatch = c->maxlen - c->tail;
		secondBatch = c->head;
	}

	if (size < firstBatch) {
		secondBatch = 0;
		firstBatch = size;
	}
	else if (size < firstBatch + secondBatch) {
		secondBatch = size - firstBatch;
	}

	int data_ix = 0;
	memcpy(data, c->buffer + c->tail, firstBatch);
	c->tail += firstBatch;
	data_ix += firstBatch;

	if (c->tail == c->maxlen)
		c->tail = 0;

	if (secondBatch > 0) {
		memcpy(data + data_ix, c->buffer, secondBatch);
		c->tail += secondBatch;
	}

	return firstBatch + secondBatch;
}


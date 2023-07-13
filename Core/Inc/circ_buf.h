/*
 * circ_buf.h
 *
 *  Created on: Jul 12, 2023
 *      Author: dominik
 */

#ifndef INC_CIRC_BUF_H_
#define INC_CIRC_BUF_H_

#include <stdint.h>

#define CIRC_BUF_DEF(x,y)                 \
    uint8_t x##_data_space[y];            \
    circ_buf_t x = {                      \
        .buffer = x##_data_space,         \
        .head = 0,                        \
        .tail = 0,                        \
        .maxlen = y                       \
    }

typedef struct {
    uint8_t * const buffer;
    int head;
    int tail;
    const int maxlen;
} circ_buf_t;

int circ_buf_push(circ_buf_t *c, uint8_t data);
int circ_buf_pop(circ_buf_t *c);
int circ_buf_push_bytes(circ_buf_t *c, uint8_t *data, int size);

#endif /* INC_CIRC_BUF_H_ */

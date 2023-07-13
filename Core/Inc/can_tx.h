/*
 * can_tx.h
 *
 *  Created on: Jul 11, 2023
 *      Author: dominik
 */

#ifndef INC_CAN_TX_H_
#define INC_CAN_TX_H_

#include <stdint.h>
#include "stm32f0xx_hal.h"

#define CAN_MAILBOX_LENGTH 24

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint8_t len;
} CAN_Tx_Msg_t;

void can_tx_set_hcan(CAN_HandleTypeDef *hcan);

void can_tx_set_title(uint8_t * buf, uint8_t len);
void can_tx_set_artist(uint8_t * buf, uint8_t len);
void can_tx_set_album(uint8_t * buf, uint8_t len);

void can_tx_send_packet(uint32_t id, uint8_t* data, uint8_t len);

#endif /* INC_CAN_TX_H_ */

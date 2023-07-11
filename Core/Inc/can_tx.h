/*
 * can_tx.h
 *
 *  Created on: Jul 11, 2023
 *      Author: dominik
 */

#ifndef INC_CAN_TX_H_
#define INC_CAN_TX_H_

#include <stdint.h>

void can_tx_set_title(uint8_t * buf, uint8_t len);
void can_tx_set_artist(uint8_t * buf, uint8_t len);
void can_tx_set_album(uint8_t * buf, uint8_t len);


#endif /* INC_CAN_TX_H_ */

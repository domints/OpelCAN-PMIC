/*
 * uart.h
 *
 *  Created on: Jul 11, 2023
 *      Author: dominik
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32f0xx_hal.h"
#include <stdbool.h>

#define UART_BUFFER_SIZE 32
#define UART_RING_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 32

void uart_start(UART_HandleTypeDef *uart);
int uart_data_available();
int uart_get_byte();
void uart_send_data(uint8_t * data, uint8_t len);

#endif /* INC_UART_H_ */

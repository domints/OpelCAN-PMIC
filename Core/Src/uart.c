#include "uart.h"
#include "can_tx.h"
#include "circ_buf.h"

uint8_t uart_byte_buf[UART_RING_BUFFER_SIZE];
UART_HandleTypeDef uart_huart;

CIRC_BUF_DEF(uart_ring_buf, UART_RING_BUFFER_SIZE);
CIRC_BUF_DEF(uart_tx_buf, UART_TX_BUFFER_SIZE);

bool uart_cts = true;

void _uart_send_buffer();

void uart_start(UART_HandleTypeDef *uart) {
	uart_huart = *uart;
	HAL_UARTEx_ReceiveToIdle_DMA(uart, uart_byte_buf, UART_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(uart->hdmarx, DMA_IT_HT); // Theoretically I might be able to disable half transfers. But maybe let's implement rest n ow.
}

int uart_data_available() {
	if (uart_ring_buf.head >= uart_ring_buf.tail)
		return uart_ring_buf.head - uart_ring_buf.tail;

	return uart_ring_buf.maxlen - (uart_ring_buf.tail - uart_ring_buf.head);
}

int uart_get_byte() {
	return circ_buf_pop(&uart_ring_buf);
}

void uart_send_data(uint8_t * data, uint8_t len) {
	HAL_UART_Transmit (&uart_huart, data, len, 100);
}



void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart->Instance == USART1) {
		uint16_t copyIx = 0;
		uint16_t copySize = Size;

		if (copySize > 0) {
			circ_buf_push_bytes(&uart_ring_buf, uart_byte_buf + copyIx,
					copySize);
		}
		HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_byte_buf, UART_BUFFER_SIZE);
		__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
	}
}



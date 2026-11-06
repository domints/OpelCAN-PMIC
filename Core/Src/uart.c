#include "uart.h"
#include "can_tx.h"
#include "circ_buf.h"

uint8_t uart_byte_buf[UART_RING_BUFFER_SIZE];
UART_HandleTypeDef* uart_huart;

CIRC_BUF_DEF(uart_ring_buf, UART_RING_BUFFER_SIZE);
CIRC_BUF_DEF(uart_tx_buf, UART_TX_BUFFER_SIZE);

bool uart_cts = true;
bool uart_error_happened = false;

void _uart_send_buffer();
void _uart_restart_rx();

void uart_start(UART_HandleTypeDef *uart) {
	uart_huart = uart;
	_uart_restart_rx();
}

void _uart_restart_rx() {
	HAL_UARTEx_ReceiveToIdle_DMA(uart_huart, uart_byte_buf, UART_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(uart_huart->hdmarx, DMA_IT_HT);
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
	HAL_UART_Transmit (uart_huart, data, len, 100);
}


void uart_loop() {
	if (uart_huart->RxState == HAL_UART_STATE_READY && uart_error_happened) {
		uart_error_happened = false;
		_uart_restart_rx();
	}
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

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	uart_error_happened = true;
}



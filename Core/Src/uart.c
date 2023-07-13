#include "uart.h"
#include "circ_buf.h"

uint8_t uart_byte_buf[UART_BUFFER_SIZE];
UART_HandleTypeDef uart_huart;

CIRC_BUF_DEF(uart_ring_buf, UART_RING_BUFFER_SIZE);

void uart_start(UART_HandleTypeDef *uart) {
	uart_huart = *uart;
	HAL_UARTEx_ReceiveToIdle_DMA(uart, uart_byte_buf, UART_BUFFER_SIZE);
}

void uart_interrupt(UART_HandleTypeDef *uart, uint16_t Size) {


}

int uart_data_available() {
	if (uart_ring_buf.head >= uart_ring_buf.tail)
		return uart_ring_buf.head - uart_ring_buf.tail;

	return uart_ring_buf.maxlen - (uart_ring_buf.tail - uart_ring_buf.head);
}

int uart_get_byte() {
	return circ_buf_pop(&uart_ring_buf);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	circ_buf_push_bytes(&uart_ring_buf, uart_byte_buf, Size);
	HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_byte_buf, UART_BUFFER_SIZE);
}


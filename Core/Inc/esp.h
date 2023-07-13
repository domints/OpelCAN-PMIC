/*
 * esp.h
 *
 *  Created on: Jul 12, 2023
 *      Author: dominik
 */

#ifndef INC_ESP_H_
#define INC_ESP_H_

#include <stdint.h>

#define CMD_SHUTDOWN 0xFE
#define CMD_PLAYPAUSE 0x01
#define CMD_NEXT 0x02
#define CMD_PREV 0x03

#define CMD_TITLE 0x81
#define CMD_ARTIST 0x82
#define CMD_ALBUM 0x83
#define CMD_TRACK_NO 0x84
#define CMD_TRACK_CNT 0x85
#define CMD_GENRE 0x86
#define CMD_TIME 0x87

#define CMD_REMOTE_SUSPEND 0x90
#define CMD_STOPPED 0x91
#define CMD_STARTED 0x92

#define UART_MODE_WAIT 0x00
#define UART_MODE_READ_SIZE 0x01
#define UART_MODE_READ_TEXT 0x02
#define UART_MODE_RESET 0xFE

void esp_reset();
void esp_receive_uart();


#endif /* INC_ESP_H_ */

/*
* file pc_uart.h
*
* Author: Luka Gavric 2018/3194
*
*/
#ifndef PCUART_H
#define PCUART_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "defines.h"

uint8_t pc_uart_rx_cmd_start(void);
void pc_uart_tx(uint8_t state);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
uint8_t pc_uart_error(void);

#endif

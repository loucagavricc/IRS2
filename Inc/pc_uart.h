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
#include "string.h"
#include "stdio.h"
#include "defines.h"

uint8_t pc_uart_check_rx(void);
void pc_uart_rx(uint8_t rx_count);

uint8_t pc_uart_rx_cmd(void);
uint8_t pc_init_lock(void);
uint8_t pc_init_lock_status(void);

uint8_t pc_uart_tx(char* string);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
uint8_t pc_uart_error(void);

#endif

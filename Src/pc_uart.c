/*
* file pc_uart.c
*
* Author: Luka Gavric 2018/3194
*
*/

#include "pc_uart.h"


extern UART_HandleTypeDef huart2;

static uint8_t tx_buffer[5] = {0,0,0,0,0};
static uint8_t tx_count = 0;
static uint8_t tx_busy = FALSE;
static uint8_t rx_byte = 0;


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (++tx_count > 2)
	{
		tx_count = 0;
	}
	else
	{
		HAL_UART_Transmit_IT(&huart2, &tx_buffer[tx_count], 1);
	}
}

uint8_t pc_uart_rx_cmd_start(void)
{
	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	if( rx_byte == PC_CMD_START )
	{
		rx_byte = 0;
		return TRUE;
	}
	else
		return TRUE;
#warning "should return FALSE, but changed for debug purposes"
		//return FALSE;
}

void pc_uart_tx(uint8_t state)
{
	switch (state)
	{
		case STATE_INIT:
			break;
		
		case STATE_FIRST_LOCK:
		case STATE_SECOND_LOCK:
		case STATE_THIRD_LOCK:
			
			break;
		
		case STATE_UNLOCK_1:
			break;
		
		case STATE_UNLOCK_2:
			break;
		
		case STATE_BLOCK:
			break;
	}
}

uint8_t pc_uart_error(void)
{
	if (tx_busy)
		return RET_FAIL;
	else
	{
		tx_buffer[0] = 'E';
		tx_buffer[1] = 'R';
		tx_buffer[2] = 'R';
		
		tx_busy = TRUE;
		
		HAL_UART_Transmit_IT(&huart2, tx_buffer, 1);
		
		return RET_SUCCESS;
	}
}

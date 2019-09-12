/*
* file pc_uart.c
*
* Author: Luka Gavric 2018/3194
*
*/

#include "pc_uart.h"

#define TX_BUFFER_SIZE	60
#define RX_BUFFER_SIZE	5

extern UART_HandleTypeDef huart2;

static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_busy = FALSE;
static uint8_t rx_buffer[RX_BUFFER_SIZE];


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	tx_busy = FALSE;
}

uint8_t pc_uart_rx_cmd(void)
{
	static uint8_t pc_uart_rx_cmd_state = 0;
	
	if( pc_uart_rx_cmd_state == 0 )
		HAL_UART_Receive_IT(&huart2, rx_buffer, 1);
	
	pc_uart_rx_cmd_state = 1;
	
	if( rx_buffer[0] == PC_CMD_START )
	{
		rx_buffer[0] = 0;
		pc_uart_rx_cmd_state = 0;
		return PC_CMD_START;
	}
	else if ( rx_buffer[0] == PC_CMD_LCKSET )
	{
		rx_buffer[0] = 0;
		pc_uart_rx_cmd_state = 0;
		return PC_CMD_LCKSET;
	}
	else
		return FALSE;
}

uint8_t pc_init_lock(void)
{
	static uint8_t pc_init_lock_state = 0;
	
	if(pc_init_lock_state == 0)
	{
		HAL_UART_Receive_IT(&huart2, &rx_buffer[1], 4);
		pc_init_lock_state = 1;
	}
	if (rx_buffer[4] == PC_CMD_LCKSET_END)
	{
		set_lock_combination(0x01 << rx_buffer[1], 
													0x01 << rx_buffer[2], 
													0x01 << rx_buffer[3]);
		pc_init_lock_state = 0;
		return RET_SUCCESS;
	}
	return RET_FAIL;
}

uint8_t pc_uart_tx(char* string_to_send)
{
	if (strlen(string_to_send) > TX_BUFFER_SIZE)
	{
		Error_Handler();
	}
	
	if (tx_busy)
		return RET_FAIL;
	
	tx_busy = TRUE;
	strcpy(tx_buffer, string_to_send);	
	HAL_UART_Transmit_IT(&huart2, tx_buffer, strlen(string_to_send));
	
	return RET_SUCCESS;
}

uint8_t pc_uart_error(void)
{
	char error_string[] = "ERROR!\r\n";
	if (tx_busy)
		return RET_FAIL;
	
	tx_busy = TRUE;
	strcpy(tx_buffer, error_string);
	HAL_UART_Transmit_IT(&huart2, tx_buffer, strlen(error_string));
	
	return RET_SUCCESS;
}

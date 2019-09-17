/*
* file pc_uart.c
*
* Author: Luka Gavric 2018/3194
*
*/

#include "pc_uart.h"

/* Buffer size defines */
#define TX_BUFFER_SIZE	256
#define RX_BUFFER_SIZE	256

/* External USART handle */
extern UART_HandleTypeDef huart2;

/* Buffers for storing outgoing and incoming USART2 data */
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t rx_buffer[RX_BUFFER_SIZE];

/* Global variables for communication */
static uint8_t tx_busy = FALSE;
static uint8_t rx_completed = FALSE;
static uint8_t lock_init_status = FALSE;

/**
* Callback when TX cycle is completed.
* @param huart USART handle
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	tx_busy = FALSE;
}

/**
* Callback when RX cycle is completed.
* @param huart USART handle
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	rx_completed = TRUE;
}

/**
* Checks if RX cycle is completed.
* @return TRUE if completed, FALSE otherwise
*/
uint8_t pc_uart_check_rx(void)
{
	return rx_completed;
}

/**
* Receive amount of data from USART2.
* @param rx_count Amount of data to be received
*/
void pc_uart_rx(uint8_t rx_count)
{	
	HAL_UART_Receive_IT(&huart2, rx_buffer, rx_count);
}

/**
* Get command message received from USART2.
* @return command code if CMD_START or CMD_LCKSET, FALSE otherwise
*/
uint8_t pc_uart_rx_cmd(void)
{
	uint8_t retval;
	
	if ( rx_completed )
	{	
		if( rx_buffer[0] == PC_CMD_START || rx_buffer[0] == PC_CMD_LCKSET)
		{
			retval = rx_buffer[0];
		}
		else
		{
			retval = FALSE;
		}
		
		rx_buffer[0] = 0;
		rx_completed = 0;
			
		return retval;
	}
	
	else
		return FALSE;
}

/**
* Initialize lock combination. Waits for 3 numbers and CMD_LCKSET_END.
*	Sets lock_init_status to FALSE if messages fail to meet protocol.
*
* @return RET_SUCCESS if all bytes received, RET_FAIL otherwise
*/
uint8_t pc_init_lock(void)
{
	if ( rx_completed )
	{
		if ( rx_buffer[3] != PC_CMD_LCKSET_END )  // invalid end byte
		{
			rx_completed = FALSE;						
			lock_init_status = FALSE;
			return RET_SUCCESS;
		}
		else
		{
			if( rx_buffer[0] > 15 || rx_buffer[1] > 15 || rx_buffer[2] > 15 ) // invalid input
				lock_init_status = FALSE;			
			
			else
			{
				set_lock_combination(0x01 << rx_buffer[0], 
															0x01 << rx_buffer[1], 
															0x01 << rx_buffer[2]);
				lock_init_status = TRUE;
			}
			return RET_SUCCESS;
		}
	}
	else
		return RET_FAIL;
}

/**
* Check initializing status of safe lock. Resets after first read.
* @return TRUE if successfully initialized, FALSE otherwise
*/
uint8_t pc_init_lock_status(void)
{
	if (lock_init_status)
	{	
		lock_init_status = FALSE;
		return TRUE;
	}
	else
		return FALSE;
}

/**
* Send data to USART2.
* @param string_to_send Char pointer to null terminated string to be sent!
* @return RET_FAIL if tx is busy, RET_SUCCESS otherwise
*/
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

/**
* Model function for sending message to USART2.
* @return RET_SUCCESS on success, RET_FAIL if tx is busy
*/
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

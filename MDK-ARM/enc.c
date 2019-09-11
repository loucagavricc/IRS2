/*
* file enc.c
*
* Author: Luka Gavric 2018/3194
*
*/

#include "enc.h"

extern SPI_HandleTypeDef hspi2;

// variables that are used for communication between function calls
static uint8_t button_pushed = FALSE;
static uint8_t encoder_rotated_left = FALSE;
static uint8_t encoder_rotated_right = FALSE;
static uint8_t unlocked = FALSE;
static uint8_t blocked = FALSE;
static uint8_t false_cnt = 0;

// main variable that stores value of encoder
static uint16_t encoder_value = 1;

// main variable for storing lock code value
static uint16_t lock_value = 0x01 << 13;

void enc_flash_success(void)
{
	uint8_t data[] = {0xFF, 0xFF};
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);																// default LED turn on
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
	
	HAL_Delay(200);
	
	data[0] = 0x00;
	data[1] = 0x00;
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);																// default LED turn on
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
	
	HAL_Delay(200);
}


void enc_flash_fail(void)
{
	uint8_t data[2] = {0x00, 0x00};
	static uint16_t data_16 = 1;
	
	if(data_16 < 0x8000) 
		data_16 *= 2;
	else
		data_16 = 1;
	
	data[0] = data_16>>8;
	data[1] = data_16;
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);															// update active LED
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
	
	HAL_Delay(100);
	
}

void enc_init(void)
{
	uint8_t data[2] = {0x0,0x1};
	
	button_pushed = FALSE;
	encoder_rotated_left = FALSE;
	encoder_rotated_right = FALSE;
	unlocked = FALSE;
	blocked = FALSE;
	false_cnt = 0;
	encoder_value = 1;
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);																// default LED turn on
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
}

void enc_get_value(uint8_t *value_of_encoder)
{
	uint8_t val_det;
	
	for( val_det = 0; val_det < 16; val_det++ )
	{
		if ( encoder_value == 0x1 << val_det)
		{
			*value_of_encoder = val_det;
		}
	}
}

uint8_t enc_event_check(void)
{
	if(button_pushed == TRUE ||
			encoder_rotated_left == TRUE ||
			encoder_rotated_right == TRUE)
	{
		return TRUE;
	}
	else
		return FALSE;
}

void enc_process_event(void)
{
	uint8_t data[2] = {0,1};
	
	if(encoder_rotated_right || 
			encoder_rotated_left )
	{
		if (encoder_rotated_right)
		{
			if(encoder_value < 0x8000) 
				encoder_value *= 2;
			else
				encoder_value = 1;
		
			encoder_rotated_right = FALSE;
		}
		else
		{
			if(encoder_value > 0x0001) 
				encoder_value /= 2;
			else
				encoder_value = 0x8000;
			
			encoder_rotated_left = FALSE;
		}		
		
		data[0] = encoder_value>>8;
		data[1] = encoder_value;
		
		HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi2, data, 2, 1000);															// update active LED
		HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);	
		
	}
	
	if (button_pushed)
	{
		if (check_valid_combination(encoder_value))
		{
			buzz_set_type(BUZZ_TYPE_SUCCESS);
			buzz(SOURCE_NOT_IT);
		}
		else
		{
			buzz_set_type(BUZZ_TYPE_FAIL);
			buzz(SOURCE_NOT_IT);
		}
		button_pushed = FALSE;
	}
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}


uint8_t check_valid_combination(uint16_t value)
{	
	if(lock_value == value)
	{
		unlocked = TRUE;
	  false_cnt = 0;
		return TRUE;
	}
	else if(false_cnt >= 5)
	{
		blocked = TRUE;
		false_cnt = 0;
		return FALSE;
	}
	else
	{
		false_cnt++;
		return FALSE;
	}
}

uint8_t enc_is_unlocked(void)
{
	uint8_t retval = unlocked;
	unlocked = FALSE; 					//multiple readout protection
	return retval;
}

uint8_t enc_is_blocked(void)
{
	uint8_t retval = blocked;
	blocked = FALSE;
	return retval;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	if (GPIO_Pin == ECA_Pin) // encoder is rotated
	{
		if (HAL_GPIO_ReadPin(ECB_GPIO_Port, ECB_Pin) == GPIO_PIN_SET)
		{
			if (HAL_GPIO_ReadPin(ECA_GPIO_Port, ECA_Pin) == GPIO_PIN_SET)
			{
				encoder_rotated_left = TRUE;
			}
			else
			{
				encoder_rotated_right = TRUE;
			}
		}
		else
		{
			if (HAL_GPIO_ReadPin(ECA_GPIO_Port, ECA_Pin) == GPIO_PIN_SET)
			{
				encoder_rotated_right = TRUE;
			}
			else
			{
				encoder_rotated_left = TRUE;
			}
		}
	}
	
	if (GPIO_Pin == EC_BTN_Pin)	// encoder button is pushed
	{
		button_pushed = TRUE;
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	}
}

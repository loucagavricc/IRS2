/*
* file enc.c
*
* Author: Luka Gavric 2018/3194
*
*/

#include "enc.h"

// external variables for peripherals
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim3;

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
static uint16_t lock_value[] = {0x01 << 13, 0x01 << 13, 0x01 << 13};

/**
* Flashing diodes on encoder click in NO RHYTHM, just turn them off for god's sake
*/
void enc_leds_off(void)
{
	uint8_t data[] = {0x0, 0x0};
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
}

/**
* Flashing diodes on encoder click in successful rhythm
*/
void enc_flash_success(void)
{
	uint8_t data[] = {0xFF, 0xFF};
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
	
	HAL_Delay(200);
	
	data[0] = 0x00;
	data[1] = 0x00;
	
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
	
	HAL_Delay(200);
}

/**
* Flashing diodes on encoder click in fail rhythm
*/
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

/**
* Initializing encoder click and static variables
*/
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

/**
* Reading encoder value from static variable that is updated on events
* @param value_of_encoder unsigned byte in range 1-16
*/
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

/**
* Checking for event stored in static variables updated on IRQ
* @return TRUE (1) if event occured, FALSE (0) otherwise
*/
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


/**
* Processes event that occured - encoder rotation or button push
* @param lock_order Unsigned byte that selects current lock
*/
void enc_process_event(uint8_t lock_order)
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
		if (check_valid_combination(encoder_value, lock_order))
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
}

/**
* Set lock combination 
* @param lock1 Value for lock1 set
* @param lock2 Value for lock2 set
* @param lock3 Value for lock3 set
*/
void set_lock_combination(uint16_t lock1, uint16_t lock2, uint16_t lock3)
{
	lock_value[0] = lock1;
	lock_value[1] = lock2;
	lock_value[2] = lock3;
}

/**
* Checking if encoder value is the same as lock combination 
* @param value Unsigned word in one-hot coding (0x0001, 0x0002, 0x0004 ... 0x4000, 0x8000)
* @param lock_order Unsigned byte that selects current lock
* @return TRUE (1) on match with set lock combination, FALSE (0) otherwise
*/
uint8_t check_valid_combination(uint16_t value, uint8_t lock_order)
{
	if(lock_value[lock_order] == value)
	{
		unlocked = TRUE;
	  false_cnt = 0;
		return TRUE;
	}
	else if(false_cnt >= BLOCK_FAIL_COUNT)
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

/**
* Check if unlocked flag is set
* @return TRUE (1) or FALSE (0)
*/
uint8_t enc_is_unlocked(void)
{
	uint8_t retval = unlocked;
	unlocked = FALSE; 					//multiple readout protection
	return retval;
}

/**
* Check if blocked flag is set
* @return TRUE (1) or FALSE (0)
*/
uint8_t enc_is_blocked(void)
{
	uint8_t retval = blocked;
	blocked = FALSE;
	return retval;
}

/**
* External interrupt callback
* @param GPIO_Pin EXTI pin set that initiated interrupt
*/
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
		debounce(SOURCE_NOT_IT);
	}
}

/**
* Debounce button on encoder click board
* @param source_of_call SOURCE_IT if called from timer callback, SOURCE_NOT_IT otherwise
*/
void debounce(uint8_t source_of_call)
{
	
	switch(source_of_call)
	{
		case SOURCE_NOT_IT:
			
			HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
			HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
			HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
		
			break;
		
		case SOURCE_IT:
			
			if (HAL_GPIO_ReadPin(EC_BTN_GPIO_Port, EC_BTN_Pin) == GPIO_PIN_SET)
				button_pushed = 1;
			HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
			HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn); //clear irqs
			HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 			//enable irqs
				
			break;
	}
	
}

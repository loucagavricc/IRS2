/*
* file buzzer.c
*
* Author: Luka Gavric 2018/3194
*
*/

#include "buzzer.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

static uint8_t buzzing_type = BUZZ_TYPE_FAIL;

void buzz_success(uint8_t source_of_call)
{
	static uint8_t state_of_playing = 0;
	
	switch (state_of_playing)
	{
		case 0:
			htim1.Init.Prescaler = 20;								//set buzzing pitch
			if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
			{
				Error_Handler();
			}
			state_of_playing = 1;
			
			htim2.Init.Period = 5000;		//set period of ~1s
			if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
			{
				Error_Handler();
			}
			
			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);		//turn on buzzer
			
			HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);	//start timer2
			break;
		case 1:
			if (source_of_call == 1)
			{
				
				HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);	//stop timer2 if source is timer2 int
				
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);	//stop buzzing
				state_of_playing = 0;
			}
			else
			{
				return;
			}
			break;
	}
}

void buzz_fail(uint8_t source_of_call)
{
	static uint8_t state_of_playing = 0;
	
	switch (state_of_playing)
	{
		case 0:
			htim1.Init.Prescaler = 10;		//set buzzing pitch
			if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
			{
				Error_Handler();
			}
		case 2:
		case 4:
		case 6:
		case 8:
			state_of_playing++;
		
			htim2.Init.Period = 500;			//set period of ~1s
			if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
			{
				Error_Handler();
			}

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);			//turn on buzzer
			HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);			//start timer2
			
			break;
			
		case 1:
		case 3:
		case 5:
		case 7:
			if (source_of_call == 1)
			{
				htim2.Init.Period = 500;				//set period of 1s
				if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
				{
					Error_Handler();
				}
				
				HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1); //restart timer2 if source is timer2 int
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);		//stop buzzing
				
				state_of_playing++;
			}
			break;
			
		case 9:
			if (source_of_call == 1)
			{

				HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);				//stop timer2 if source is timer2 int
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);				//stop buzzing
				
				state_of_playing = 0;
			}
			break;
	}
}

void toggle_pwm(void)
{
	static uint8_t timer_status = 0;
	
	if (timer_status == 0)
	{
		timer_status = 1;
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);		//start pwm
	}
	else
	{
		timer_status = 0;
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);		//stop pwm
	}
}


void buzz(uint8_t source)
{
	if (buzzing_type == BUZZ_TYPE_SUCCESS)
		buzz_success(source);
	else
		buzz_fail(source);
}

void buzz_set_type(uint8_t type)
{
	buzzing_type = type;
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	buzz(SOURCE_IT);
}

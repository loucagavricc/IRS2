/*
* file buzzer.c
*
* Author: Luka Gavric 2018/3194
*
*/
#ifndef BUZZER_C
#define BUZZER_C

#include "stm32f1xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "defines.h"

/*
* Function for toggling PWM pin ON/OFF
*/
void toggle_pwm(void);
void buzz_success(uint8_t source_of_call);
void buzz_fail(uint8_t source_of_call);
void buzz(uint8_t source);

void buzz_set_type(uint8_t type);

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);

#endif

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

/*
* Function for toggling PWM pin ON/OFF
*/
void toggle_pwm(void);
void buzz_success(uint8_t source_of_call);
void buzz_fail(uint8_t source_of_call);
void buzz(uint8_t source);

#endif

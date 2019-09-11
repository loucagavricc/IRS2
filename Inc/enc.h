/*
* file enc.h
*
* Author: Luka Gavric 2018/3194
*
*/

#ifndef ENC_C
#define ENC_C

#include "stm32f1xx_hal.h"
#include "defines.h"
#include "spi.h"
#include "gpio.h"
#include "buzzer.h"

void enc_flash_success(void);
void enc_flash_fail(void);

void enc_get_value(uint8_t *value_of_encoder);

void enc_init(void);
uint8_t enc_event_check(void);
void enc_process_event(void);

uint8_t check_valid_combination(uint16_t value);

uint8_t enc_is_unlocked(void);
uint8_t enc_is_blocked(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif

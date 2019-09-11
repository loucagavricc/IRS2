/*
* file defines.h
*
* Author: Luka Gavric 2018/3194
*
*/
#ifndef DEFINES_H
#define DEFINES_H

#include "stm32f1xx_hal.h"
#include "tim.h"
#include "gpio.h"

// MACROs for code size optimization

// MACROs for boolean return type
#define TRUE	(uint8_t) 1
#define FALSE	(uint8_t) 0

// MACROs for function return values
#define RET_SUCCESS (uint8_t) 1
#define RET_FAIL		(uint8_t) 0

// MACROs for param defining if buzzing is called from IT callback
#define SOURCE_NOT_IT		(uint8_t) 0
#define SOURCE_IT				(uint8_t) 1

// MACROs for defining type of buzzing
#define BUZZ_TYPE_SUCCESS		(uint8_t) 1
#define BUZZ_TYPE_FAIL			(uint8_t) 0

// MACROs to be used as a state machine states
#define STATE_INIT				(uint8_t) 0
#define STATE_FIRST_LOCK	(uint8_t) 1
#define STATE_SECOND_LOCK	(uint8_t) 2
#define STATE_THIRD_LOCK	(uint8_t) 3
#define STATE_UNLOCK_1		(uint8_t) 4
#define STATE_UNLOCK_2		(uint8_t) 5
#define STATE_BLOCK				(uint8_t) 6

// MACROs for PC commands received by UART
#define PC_CMD_START			(uint8_t) 0xA5

//template for function commenting

/**
* a normal member taking two arguments and returning an integer value.
* @param a an integer argument.
* @param s a constant character pointer.
* @see Javadoc_Test()
* @see ~Javadoc_Test()
* @see testMeToo()
* @see publicVar()
* @return The test results
*/
void debounce(uint8_t source_of_call);

#endif

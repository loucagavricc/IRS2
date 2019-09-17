/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "enc.h"
#include "buzzer.h"
#include "pc_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	static uint8_t state = STATE_RESET;
	uint32_t timestamp;
	char message[50];
	uint8_t encoder_value;
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */	
	
	while (1)
	{
/****** MAIN STATE MACHINE ***************/		
		switch (state)			
		{
/******** RESET **************************/			
			case STATE_RESET:
				enc_leds_off();
				while( pc_uart_tx(
					"\r\n"
					"-----------\r\n"
					"Luka Gavric\r\n"
					"IRS2 \r\n"
					"-----------\r\n\r\n"
					"165 - start\r\n"
					"185 - set combination\r\n"
					"241 - end set comb\r\n\r\n") == RET_FAIL);
				pc_uart_rx(1);
				state = STATE_INIT;
				break;
				
/******** INITIALIZING *******************/			
			case STATE_INIT:
				if ( pc_uart_check_rx() == FALSE )
					break;
				else
				{
					switch ( pc_uart_rx_cmd() )
					{
						case PC_CMD_START:
							enc_init();
							state = STATE_FIRST_LOCK;
							pc_uart_tx("Lock 1: ");
							break;
						
						case PC_CMD_LCKSET:
							pc_uart_rx(4);
							while ( pc_uart_tx("Setting lock combination.\r\n") == RET_FAIL );
							state = STATE_SET_COMB;
							break;
						
						default:
							pc_uart_rx(1);
							state = STATE_INIT;
							break;
					}
				}
				
				break;
				
/******* SETTING LOCK COMBINATION ********/				
			case STATE_SET_COMB:
				if (pc_init_lock())
				{
					
					if ( pc_init_lock_status())
						while ( pc_uart_tx("Successful lock combination set!\r\n") == RET_FAIL );
					else
						while ( pc_uart_tx("Unsuccessful lock combination set!\r\n"
																"Number must be <16.\r\n"
																"Previous (default) combination saved.\r\n\r\n") == RET_FAIL );
					
					state = STATE_INIT;
					pc_uart_rx(1);
				}
				break;
				
/******* UNLOCKING LOCK BY LOCK **********/				
			case STATE_FIRST_LOCK:
			case STATE_SECOND_LOCK:
			case STATE_THIRD_LOCK:
					
				if (enc_event_check()) // check for button push or encoder rotation
				{
					enc_process_event(state-STATE_FIRST_LOCK);	// process event that happened
					
					enc_get_value(&encoder_value);
					
					sprintf(message, "\rLock %d: %2d", state-STATE_FIRST_LOCK+1, encoder_value);
					while( pc_uart_tx(message) == RET_FAIL );
					
					if (enc_is_unlocked())	// if successfully unlocked, move on
					{
						state++;
						while ( pc_uart_tx(" successfully unlocked\r\n") == RET_FAIL );
						if (state != STATE_UNLOCK_1)
						{
							sprintf(message, "\rLock %d: %2d", state-STATE_FIRST_LOCK+1, encoder_value);
							while( pc_uart_tx(message) == RET_FAIL );
						}
					}
					
					else if (enc_is_blocked())	// if limit of number of failed attemts reached
					{
						state = STATE_BLOCK_1;
					}
				}
			
				break;

/******** UNLOCKED PREPARATIONS **********/				
			case STATE_UNLOCK_1:
				HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
				while ( pc_uart_tx("\r\nUNLOCKED!\r\n") == RET_FAIL );
				timestamp = HAL_GetTick();
				state = STATE_UNLOCK_2;

/******** UNLOCKED ***********************/			
			case STATE_UNLOCK_2:
				enc_flash_success();
				if (HAL_GetTick() - timestamp > UNLOCK_PERIOD)
				{
					state = STATE_RESET;
					HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
				}
				
				break;

/******* BLOCKED PREPARATIONS ************/				
			case STATE_BLOCK_1:
				HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
				while ( pc_uart_tx("\r\nBLOCKED!\r\n") == RET_FAIL );
				timestamp = HAL_GetTick();
				state = STATE_BLOCK_2;
			
/******** BLOCKED ************************/			
			case STATE_BLOCK_2:
				enc_flash_fail();
				if (HAL_GetTick() - timestamp > BLOCK_PERIOD)
				{
					state = STATE_RESET;
					HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
				}
				
				break;
		}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	
	while(1)
	{
		pc_uart_error();			// reset needed
		HAL_Delay(1000);
	}
	
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

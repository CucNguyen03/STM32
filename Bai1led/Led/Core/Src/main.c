/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "st25r95.h"
#include <stdlib.h>


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
uint8_t RxChar;
uint8_t u8_RxData;
uint8_t u8_TxBuff[] = "Hello hello!!\r\n";

volatile st25r95_handle reader_handler;

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
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
volatile st25r95_handle reader_handler;
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void pwm_set_duty(TIM_HandleTypeDef *htim, uint32_t Channel, uint8_t duty)
{
	uint32_t ccr;
	if(duty >90)
	{
		duty = 90;
	}
	ccr = ((htim->Instance->ARR + 1)*duty)/100;
	__HAL_TIM_SET_COMPARE(htim, Channel, ccr);
}


void reader_irq_pulse() {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
  HAL_Delay(8);
}

void reader_nss(uint8_t enable)
{
    HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, enable ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void reader_tx(uint8_t *data, size_t len) {
  HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
}

void reader_rx(uint8_t *data, size_t len) {
  HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == RFID_IRQ_Pin)
    {
			reader_handler.irq_flag = 1;
			HAL_GPIO_TogglePin(Led4_GPIO_Port, Led4_Pin);
    }
}

void st25_card_callback(uint8_t *uid)
{
    char msg[64];
    sprintf(msg, "UID: %02X %02X %02X %02X\r\n",uid[0], uid[1], uid[2], uid[3]);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	HAL_UART_Transmit(&huart1, u8_TxBuff, sizeof(u8_TxBuff)-1, 100);
	HAL_UART_Receive_IT(&huart1, &RxChar, 1);
	 
	HAL_GPIO_WritePin(RFID_PWR_EN_GPIO_Port, RFID_PWR_EN_Pin,  GPIO_PIN_SET);
	HAL_Delay(50);
	
	 /* Setup all protocol */
  /* Setup all protocol */
  reader_handler.protocol = ST25_PROTOCOL_14443A;
  reader_handler.tx_speed = ST25_26K_106K;
  reader_handler.rx_speed = ST25_26K_106K;
  reader_handler.timerw = 0x58;
  reader_handler.ARC = 0xD1;
  reader_handler.irq_flag = 0;

  /* Bind BSP Functions */
  reader_handler.nss = reader_nss;
  reader_handler.tx = reader_tx;
  reader_handler.rx = reader_rx;
  reader_handler.irq_pulse = reader_irq_pulse;
  reader_handler.callback = st25_card_callback;
	
	st25r95_reset(&reader_handler);

	HAL_Delay(10);
	st25r95_init(&reader_handler);
	if(st25r95_IDN(&reader_handler) == ST25_OK)
	{
			char txt[] = "ST25R95 OK\r\n";
			HAL_UART_Transmit(&huart1,(uint8_t*)txt, sizeof(txt)-1,100);
	}
	else
	{
			char txt[] = "ST25R95 FAIL\r\n";
			HAL_UART_Transmit(&huart1,(uint8_t*)txt,sizeof(txt)-1,100);
	}
	
  st25r95_init(&reader_handler);
	
  st25r95_calibrate(&reader_handler);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  st25r95_idle(&reader_handler);
	
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_GPIO_WritePin(Led0_GPIO_Port, Led0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Led1_GPIO_Port, Led1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Led2_GPIO_Port, Led2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Led3_GPIO_Port, Led3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Led4_GPIO_Port, Led4_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		
		HAL_GPIO_WritePin(Led0_GPIO_Port, Led0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Led1_GPIO_Port, Led1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Led2_GPIO_Port, Led2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Led3_GPIO_Port, Led3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Led4_GPIO_Port, Led4_Pin, GPIO_PIN_SET);
		HAL_Delay(500);
		
		HAL_GPIO_WritePin(Led0_GPIO_Port, Led0_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(Led0_GPIO_Port, Led0_Pin, GPIO_PIN_SET);
		
		HAL_GPIO_WritePin(Led1_GPIO_Port, Led1_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(Led1_GPIO_Port, Led1_Pin, GPIO_PIN_SET);
		
		HAL_GPIO_WritePin(Led2_GPIO_Port, Led2_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(Led2_GPIO_Port, Led2_Pin, GPIO_PIN_SET);
		
		HAL_GPIO_WritePin(Led3_GPIO_Port, Led3_Pin, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(Led3_GPIO_Port, Led3_Pin, GPIO_PIN_SET);
		

		
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

    pwm_set_duty(&htim1, TIM_CHANNEL_1, 20); 
    HAL_Delay(1000);

    pwm_set_duty(&htim1, TIM_CHANNEL_1, 0); 
    HAL_Delay(1000);
		
		st25r95_service(&reader_handler);
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */

static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */



/* USER CODE BEGIN 4 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
	if(huart->Instance == USART1)
	{
		HAL_UART_Transmit(&huart1, u8_TxBuff, sizeof(u8_TxBuff), 100);
	}
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_TxCpltCallback can be implemented in the user file.
   */
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */ 

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
			HAL_UART_Receive_IT(&huart1, &RxChar, 1);
      HAL_UART_Transmit(&huart1, &RxChar, 1, 100);
    }
}
/* USER CODE END 3 */
/**
  * @brief System Clock Configuration
  * @retval None
  */


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 15;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 499;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Led4_GPIO_Port, Led4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Led0_Pin|Led1_Pin|Led2_Pin|Led3_Pin|RFID_PWR_EN_Pin, GPIO_PIN_RESET);
	
	/*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET);
	
  /*Configure GPIO pin : Led4_Pin */
  GPIO_InitStruct.Pin = Led4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Led4_GPIO_Port, &GPIO_InitStruct);

 /*Configure GPIO pins : Led0_Pin Led1_Pin Led2_Pin Led3_Pin SPI1_NSS_Pin RFID_PWR_EN_Pin */
  GPIO_InitStruct.Pin = Led0_Pin|Led1_Pin|Led2_Pin|Led3_Pin|SPI1_NSS_Pin|RFID_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
	/*Configure GPIO pin : RFID_IRQ_Pin */
  GPIO_InitStruct.Pin = RFID_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RFID_IRQ_GPIO_Port, &GPIO_InitStruct);
	
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	
  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
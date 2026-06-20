/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Led4_Pin GPIO_PIN_15
#define Led4_GPIO_Port GPIOC
#define Led0_Pin GPIO_PIN_0
#define Led0_GPIO_Port GPIOA
#define Led1_Pin GPIO_PIN_1
#define Led1_GPIO_Port GPIOA
#define Led2_Pin GPIO_PIN_2
#define Led2_GPIO_Port GPIOA
#define Led3_Pin GPIO_PIN_3
#define Led3_GPIO_Port GPIOA
#define SPI1_NSS_Pin GPIO_PIN_4
#define SPI1_NSS_GPIO_Port GPIOA
#define RFID_IRQ_Pin GPIO_PIN_11
#define RFID_IRQ_GPIO_Port GPIOA
#define RFID_PWR_EN_Pin GPIO_PIN_12
#define RFID_PWR_EN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define LED_1_On()		  (HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET))
#define LED_1_Off()		  (HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET))
#define LED_2_On()		  (HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET))
#define LED_2_Off()		  (HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET))
#define LED_2_Toggle()				(HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin))
#define LED_3_On()		  (HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET))
#define LED_3_Off()		  (HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET))
#define LED_3_Toggle()				(HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin))
#define LED_4_On()		  (HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET))
#define LED_4_Off()		  (HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET))
#define LED_5_On()		  (HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, GPIO_PIN_RESET))
#define LED_5_Off()		  (HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, GPIO_PIN_SET))
#define LED_5_Toggle()				(HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin))
#define	NFC_ModulePowerEnable()		HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET)
#define	NFC_ModulePowerDisable()	HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET)

#define NUMBER_OF_BYTE_DRIVER_INFO_STORE_IN_NFC_CARD		64
#define ISO15693_NBBYTE_UID	 														0x08
typedef struct
{
	uint8_t Detected;
	uint8_t UID[ISO15693_NBBYTE_UID];
	uint8_t DriverInfoMemory[NUMBER_OF_BYTE_DRIVER_INFO_STORE_IN_NFC_CARD];
}NfcCardInfo_Typedef;

void Debug_Show(char  buffer[128]);
#define DRIVER_LICENSE_NUMBER_FIELD_LEN				13		      //15 min 12 
#define DRIVER_NAME_FIELD_LEN_FULL					  43					// full lenght as standard: 43
#define DRIVER_NAME_FIELD_LEN_SHORTEN					20	
#define SYSTEM_PARAMETER_INTERNAL_FLASH_ADD   0x08007FE0
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

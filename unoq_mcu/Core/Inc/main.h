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
#include "stm32u5xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WaterDetectEn_Pin GPIO_PIN_4
#define WaterDetectEn_GPIO_Port GPIOB
#define StatLed_Pin GPIO_PIN_3
#define StatLed_GPIO_Port GPIOB
#define WaterStatusLed_Pin GPIO_PIN_12
#define WaterStatusLed_GPIO_Port GPIOA
#define WaterDetect_Pin GPIO_PIN_7
#define WaterDetect_GPIO_Port GPIOA
#define UsbPwrDetect_Pin GPIO_PIN_10
#define UsbPwrDetect_GPIO_Port GPIOB
#define Wkp_Pin GPIO_PIN_2
#define Wkp_GPIO_Port GPIOB
#define WaterMfgTestEn_Pin GPIO_PIN_11
#define WaterMfgTestEn_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

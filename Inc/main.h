/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_system.h"

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
#define ROW_SEL_EN_GLK_Pin GPIO_PIN_9
#define ROW_SEL_EN_GLK_GPIO_Port GPIOB
#define PI_CLK_RX_Pin GPIO_PIN_0
#define PI_CLK_RX_GPIO_Port GPIOA
#define EDGE_EN_Pin GPIO_PIN_1
#define EDGE_EN_GPIO_Port GPIOA
#define PI_DATA_RX_Pin GPIO_PIN_4
#define PI_DATA_RX_GPIO_Port GPIOA
#define SEL_READ_WRITE_Pin GPIO_PIN_5
#define SEL_READ_WRITE_GPIO_Port GPIOA
#define EXT1_Pin GPIO_PIN_7
#define EXT1_GPIO_Port GPIOA
#define EXT2_Pin GPIO_PIN_0
#define EXT2_GPIO_Port GPIOB
#define ADC2_Pin GPIO_PIN_1
#define ADC2_GPIO_Port GPIOB
#define ADC1_Pin GPIO_PIN_2
#define ADC1_GPIO_Port GPIOB
#define ROW_SEL1_Pin GPIO_PIN_9
#define ROW_SEL1_GPIO_Port GPIOA
#define ROW_SEL2_Pin GPIO_PIN_6
#define ROW_SEL2_GPIO_Port GPIOC
#define ROW_SEL3_Pin GPIO_PIN_10
#define ROW_SEL3_GPIO_Port GPIOA
#define ROW_SEL_EN_Pin GPIO_PIN_11
#define ROW_SEL_EN_GPIO_Port GPIOA
#define LED_LATCH_Pin GPIO_PIN_12
#define LED_LATCH_GPIO_Port GPIOA
#define LED_RAM_CLK_TX_Pin GPIO_PIN_3
#define LED_RAM_CLK_TX_GPIO_Port GPIOB
#define SEL_MEM_LED_Pin GPIO_PIN_4
#define SEL_MEM_LED_GPIO_Port GPIOB
#define LED_RAM_DATA_TX_Pin GPIO_PIN_5
#define LED_RAM_DATA_TX_GPIO_Port GPIOB
#define EDGE_DATA_Pin GPIO_PIN_8
#define EDGE_DATA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

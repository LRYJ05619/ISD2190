/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32l1xx_hal.h"

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
#define T1_AN_Pin GPIO_PIN_0
#define T1_AN_GPIO_Port GPIOC
#define T2_AN_Pin GPIO_PIN_1
#define T2_AN_GPIO_Port GPIOC
#define T3_AN_Pin GPIO_PIN_2
#define T3_AN_GPIO_Port GPIOC
#define T4_AN_Pin GPIO_PIN_3
#define T4_AN_GPIO_Port GPIOC
#define T5_AN_Pin GPIO_PIN_0
#define T5_AN_GPIO_Port GPIOA
#define T6_AN_Pin GPIO_PIN_1
#define T6_AN_GPIO_Port GPIOA
#define T7_AN_Pin GPIO_PIN_4
#define T7_AN_GPIO_Port GPIOA
#define T8_AN_Pin GPIO_PIN_5
#define T8_AN_GPIO_Port GPIOA
#define T9_AN_Pin GPIO_PIN_6
#define T9_AN_GPIO_Port GPIOA
#define T10_AN_Pin GPIO_PIN_7
#define T10_AN_GPIO_Port GPIOA
#define T12_AN_Pin GPIO_PIN_4
#define T12_AN_GPIO_Port GPIOC
#define VM2_RST_Pin GPIO_PIN_5
#define VM2_RST_GPIO_Port GPIOC
#define T11_AN_Pin GPIO_PIN_0
#define T11_AN_GPIO_Port GPIOB
#define VM1_RST_Pin GPIO_PIN_1
#define VM1_RST_GPIO_Port GPIOB
#define T13_AN_Pin GPIO_PIN_12
#define T13_AN_GPIO_Port GPIOB
#define T14_AN_Pin GPIO_PIN_13
#define T14_AN_GPIO_Port GPIOB
#define T15_AN_Pin GPIO_PIN_14
#define T15_AN_GPIO_Port GPIOB
#define T16_AN_Pin GPIO_PIN_15
#define T16_AN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define u8 uint8_t
#define u16 uint16_t

#define MAX_DATA_LENGTH 255
#define ADC_CHANCEL_NUM 16

typedef struct SENSOR_InfoTypeDef{
    u8 sensor_type; //传感器类型

    u16 cancel_addr; //占用通道

    int8_t init_temp; //初始温度
    u16 init_freq[9]; //初始频率 使用时需要除以10

    int8_t temp; //测量温度
    u16 *freq; //测量频率 使用时需要除以10

    u8 cancel_size; //占用通道数量

    double Calculate; //计算值

    u8 para_size; //参数数量
    u16 para[4]; //参数

    u8 status; //状态位 0x00未使用 0x01主通道 0x02从通道
}Sensor_Info;




#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

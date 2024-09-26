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

#define VM_BLE_RX_BUFFER_SIZE 32
#define ADC_CHANCEL_NUM 16

typedef struct SensorInfoTypeDef{
    u8 serial[5]; //ʮλ���

    u8 sensor_type; //����������

    u8 channel_size; //ռ��ͨ������
    u16 channel_addr; //ռ��ͨ��

    int16_t init_temp; //��ʼ�¶�
    u16 init_freq[9]; //��ʼƵ�� ʹ��ʱ��Ҫ����10

    int16_t temp; //�����¶�
    u16 freq[9]; //����Ƶ�� ʹ��ʱ��Ҫ����10

    float Calculate; //����ֵ

    u8 para_size; //��������
    int32_t para[4]; //����

    u8 status; //״̬λ 0x00δʹ�� 0x01��ͨ�� 0x02��ͨ��
    u8 freq_status; //Ƶ���Ƿ�Ϊ0 ��Ϊ0����1
}SensorInfo;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

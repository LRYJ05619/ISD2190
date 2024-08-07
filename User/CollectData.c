//
// Created by ifasten on 2024/3/25.
//

#include "CollectData.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "adc.h"
#include <string.h>
#include "VMxx.h"
#include "hardware.h"
#include "temp.h"
#include "bt4531.h"
#include "FreeRTOS.h"
#include "queue.h"

extern volatile u8 VM_ERR;
extern volatile u8 Scan_Start;
extern volatile u8 restart;

extern u8 ble_flag;
extern u8 ble_len;
extern u8 BleBuf[MAX_DATA_LENGTH];

extern u8 VM1_Busy;
extern u8 VM2_Busy;
extern u8 VM1_init;
extern u8 VM2_init;

extern u16 ADC_Value[ADC_CHANCEL_NUM];
extern int8_t Temp_Value[ADC_CHANCEL_NUM];

extern SensorInfo Sensor[16];

extern QueueHandle_t usart2Queue;
extern QueueHandle_t usart3Queue;
extern QueueHandle_t usart5Queue;

void Data_Collect() {
    HAL_TIM_Base_Start_IT(&htim2);
    Scan_VM(huart2);
    Scan_VM(huart3);

    VM1_Busy = 1;
    VM2_Busy = 1;
    while (VM1_Busy && VM2_Busy && !restart) {
        if (VM_ERR) {
            restart = 1;
            break;
        }
    }
    HAL_TIM_Base_Stop_IT(&htim2); // 接收处理完成
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    if (restart) {
        VM_init = 0;
        StatuCallback(0x70, 0x13);
        return;
    }

    HAL_ADC_Start_DMA(&hadc, (uint32_t *) ADC_Value, ADC_CHANCEL_NUM);
    HAL_Delay(200);
//    ADC_Busy = 1;
//    while (ADC_Busy && !restart) {
//        if (VM_ERR) {
//            restart = 1;
//            break;
//        }
//    }

    calcuTemps(ADC_Value, Temp_Value, ADC_CHANCEL_NUM);

    for (u8 i = 0; i < 16; i++) {
        if (!Sensor[i].status)
            continue;

        Sensor[i].temp = Temp_Value[i];

        switch (Sensor[i].sensor_type) {
            case 0xA1://土压力盒
                Sensor[i].Calculate = TYLH_YL_MPa(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0],
                                                  Sensor[i].temp, Sensor[i].para[0]);
                break;
            case 0xA2://渗压计
                Sensor[i].Calculate = SYJ_YL_MPa(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0],
                                                 Sensor[i].temp, Sensor[i].para[0], Sensor[i].para[1],
                                                 Sensor[i].para[2], Sensor[i].para[3]);
                break;
            case 0xA3://钢筋计
                Sensor[i].Calculate = GJJ_YL(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0],
                                             Sensor[i].temp, Sensor[i].para[0]);
                break;
            case 0xA4://埋入式应变计
                Sensor[i].Calculate = MRYBJ_YB(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0],
                                               Sensor[i].temp);
                break;
            case 0xA5://表面应变计
                Sensor[i].Calculate = BMYBJ_YB(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0],
                                               Sensor[i].temp);
                break;
            case 0xA6://锚索计
                for (u8 k = 0, j = 0; j < 16; j++) {
                    if (Sensor[i].channel_addr & (1 << j)) {
                        Sensor[i].freq[k++] = Sensor[j].freq[0];
                    }
                }
                Sensor[i].Calculate = MSJ_YL_KN(Sensor[i].init_freq, Sensor[i].init_temp, Sensor[i].freq,
                                                Sensor[i].temp, Sensor[i].channel_size, Sensor[i].para[0],
                                                Sensor[i].para[1]);
                break;
        }
    }
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t received_char;

    if (huart->Instance == USART2) {
        received_char = (uint8_t)(huart->Instance->DR & 0xFF);
        xQueueSendFromISR(usart2Queue, &received_char, &xHigherPriorityTaskWoken);
        HAL_UART_Receive_IT(&huart2, &received_char, 1); // 重新使能接收中断
    } else if (huart->Instance == USART3) {
        received_char = (uint8_t)(huart->Instance->DR & 0xFF);
        xQueueSendFromISR(usart3Queue, &received_char, &xHigherPriorityTaskWoken);
        HAL_UART_Receive_IT(&huart3, &received_char, 1); // 重新使能接收中断
    } else if (huart->Instance == UART5) {
        received_char = (uint8_t)(huart->Instance->DR & 0xFF);
        xQueueSendFromISR(usart5Queue, &received_char, &xHigherPriorityTaskWoken);
        HAL_UART_Receive_IT(&huart5, &received_char, 1); // 重新使能接收中断
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        HAL_TIM_Base_Stop_IT(&htim2);
        __HAL_TIM_SET_COUNTER(&htim2, 0);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        HAL_TIM_Base_Stop_IT(&htim2);
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        VM_ERR = 1;
        VM1_Busy = 0;
        VM2_Busy = 0;
        VM_init = 0;
        Scan_Start = 0;
    }
    if (htim->Instance == TIM3) {
        HAL_TIM_Base_Stop_IT(&htim3); // 计数清零 重启定时器
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        ble_flag = 1;
    }
    if (htim->Instance == TIM7) {
        HAL_IncTick();
    }
}

//
// Created by ifasten on 2024/3/25.
//

#include "CollectData.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "adc.h"
#include "VMxx.h"
#include "hardware.h"
#include "temp.h"
#include "bt4531.h"
#include "FreeRTOS.h"
#include "queue.h"

extern volatile u8 VM_ERR;
extern volatile u8 Scan_Start;

extern volatile u8 ble_flag;
extern u8 ble_len;
extern u8 BleBuf[VM_BLE_RX_BUFFER_SIZE];

extern u8 Uart2Buf[VM_BLE_RX_BUFFER_SIZE];
extern u8 Uart3Buf[VM_BLE_RX_BUFFER_SIZE];

extern volatile u8 VM1_Busy;
extern volatile u8 VM2_Busy;
extern volatile u8 VM1_Init;
extern volatile u8 VM2_Init;
extern volatile u8 VM1_OK;
extern volatile u8 VM2_OK;

extern u16 ADC_Value[ADC_CHANCEL_NUM];
extern int16_t Temp_Value[ADC_CHANCEL_NUM];

extern SensorInfo Sensor[16];

extern QueueHandle_t usart2Queue;
extern QueueHandle_t usart3Queue;

void Data_Collect() {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start_IT(&htim2);
    Scan_VM(huart2);
    HAL_Delay(200);
    Scan_VM(huart3);
    VM1_OK = 0;
    VM2_OK = 0;

    VM1_Busy = 1;
    VM2_Busy = 1;

    while (VM1_Busy || VM2_Busy) ;
    VM1_OK = 0;
    VM2_OK = 0;

    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Stop_IT(&htim2);

    if (VM_ERR) {
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

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (huart->Instance == USART2) {
        // 数据包校验通过
        xQueueSendFromISR(usart2Queue, &Size, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2Buf, VM_BLE_RX_BUFFER_SIZE); // 重新使能接收中断
    }
    if (huart->Instance == USART3) {
        // 数据包校验通过
        xQueueSendFromISR(usart3Queue, &Size, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3Buf, VM_BLE_RX_BUFFER_SIZE); // 重新使能接收中断
    }
    if (huart->Instance == UART5) {
        ble_flag = 1;
        ble_len = Size;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart5, BleBuf, VM_BLE_RX_BUFFER_SIZE); // 重新使能接收中断
    }
}




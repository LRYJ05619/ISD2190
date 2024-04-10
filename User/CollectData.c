//
// Created by ifasten on 2024/3/25.
//

#include "CollectData.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "adc.h"
#include <malloc.h>
#include <string.h>
#include "VMxx.h"
#include "hardware.h"
#include "temp.h"

extern volatile  u8  VM_Busy;
extern volatile  u8  ADC_Busy;
extern volatile  u8  VM_ERR;
extern volatile  u8  Scan_Start;
extern volatile  u8 restart;

extern u8  rxdata;
extern u8  rx_check;
extern u8  rx_index;
extern u8  rx_buffer[MAX_DATA_LENGTH];

extern u8  receiving;
extern u8  VM_init;

extern u16 ADC_Value[ADC_CHANCEL_NUM];
extern int8_t Temp_Value[ADC_CHANCEL_NUM];

extern Sensor_Info Sensor[16];

void Data_Collect(){
    HAL_TIM_Base_Start_IT(&htim2);
    Scan_VM(huart3);
    VM_Busy = 1;
    while (VM_Busy && !restart) {
        if (VM_ERR) {
            restart = 1;
            break;
        }
    }
    if (restart) {
        VM_init = 0;
        //Todo 报错
        return;
    }

    for (u8 i = 4; i < 20; i += 2) {
        // 合并频率值
        Sensor[i / 2 + 6].freq[0] = ((uint16_t)rx_buffer[i] << 8) | rx_buffer[i + 1];
    }
    memset(rx_buffer, 0, MAX_DATA_LENGTH);

    HAL_TIM_Base_Start_IT(&htim2);
    Scan_VM(huart2);
    VM_Busy = 1;
    while (VM_Busy && !restart) {
        if (VM_ERR) {
            restart = 1;
            break;
        }
    }
    if (restart) {
        VM_init = 0;
        //Todo 报错
        return;
    }

    for (u8 i = 4; i < 20; i += 2) {
        // 合并频率值
        Sensor[i / 2 - 2].freq[0] = ((uint16_t)rx_buffer[i] << 8) | rx_buffer[i + 1];
    }
    memset(rx_buffer, 0, MAX_DATA_LENGTH);

    HAL_TIM_Base_Start_IT(&htim2);
    HAL_ADC_Start_DMA(&hadc, (uint32_t *) ADC_Value, ADC_CHANCEL_NUM);
    ADC_Busy = 1;
    while (ADC_Busy);

    calcuTemps(ADC_Value, Temp_Value, ADC_CHANCEL_NUM);

    for (u8 i = 0; i < 16; i ++) {
        if(!Sensor[i].status)
            continue;

        Sensor[i].temp = Temp_Value[i];

        switch (Sensor[i].sensor_type) {
            case 0xA1://土压力盒
                Sensor[i].Calculate = TYLH_YL_MPa(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0], Sensor[i].temp, Sensor[i].para1);
                break;
            case 0xA2://渗压计
                Sensor[i].Calculate = SYJ_YL_MPa(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0], Sensor[i].temp, Sensor[i].para1, Sensor[i].para2, Sensor[i].para3, Sensor[i].para4);
                break;
            case 0xA3://钢筋计
                Sensor[i].Calculate = GJJ_YL(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0], Sensor[i].temp, Sensor[i].para1);
                break;
            case 0xA4://埋入式应变计
                Sensor[i].Calculate = MRYBJ_YB(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0], Sensor[i].temp);
                break;
            case 0xA5://表面应变计
                Sensor[i].Calculate = BMYBJ_YB(Sensor[i].init_freq[0], Sensor[i].init_temp, Sensor[i].freq[0], Sensor[i].temp);
                break;
            case 0xA6://锚索计
                Sensor[i].freq = (u16 *)malloc(Sensor[i].size * sizeof(u16));

                for(u8 k = 0, j = 0; j < 16; ++j){
                    if(Sensor[i].cancel_addr && (1 << j)){
                        Sensor[i].freq[k++] = Sensor[j].freq[0];
                    }
                }
                Sensor[i].Calculate = MSJ_YL_KN(Sensor[i].init_freq, Sensor[i].init_temp, Sensor[i].freq, Sensor[i].temp, Sensor[i].size, Sensor[i].para1, Sensor[i].para2);
                break;
        }
    }
}
int i = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART3){
        if(0xAA == rxdata){
            receiving = 1;
        }
        if(receiving){
            rx_buffer[rx_index] = rxdata;
            if(rx_check == rxdata){
                if(0xAA != rxdata) {
                    if (0x05 == rx_buffer[3]) {
                        VM_init = 1;
                        memset(rx_buffer, 0, MAX_DATA_LENGTH);
                    }
                    if (0x73 == rx_buffer[3]) {
                        Scan_Start = 0;
                    }
                    rx_index = 0;
                    receiving = 0;
                    rx_check = 0;
                    VM_Busy = 0;
                    HAL_UART_Receive_IT(&huart3, &rxdata, 1);
                    HAL_TIM_Base_Stop_IT(&htim2);
                    __HAL_TIM_SET_COUNTER(&htim2, 0);
                    return;
                }
            }
            rx_check += rxdata;
            rx_index++;
        }
        HAL_UART_Receive_IT(&huart3, &rxdata, 1);
    };
    if(huart->Instance == USART2){
        if(0xAA == rxdata){
            receiving = 1;
        }
        if(receiving){
            rx_buffer[rx_index] = rxdata;
            if(rx_check == rxdata){
                if(0xAA != rxdata) {
                    if (0x05 == rx_buffer[3]) {
                        VM_init = 1;
                        memset(rx_buffer, 0, MAX_DATA_LENGTH);
                    }
                    if (0x73 == rx_buffer[3]) {
                        Scan_Start = 0;
                    }
                    rx_index = 0;
                    receiving = 0;
                    rx_check = 0;
                    VM_Busy = 0;
                    HAL_UART_Receive_IT(&huart3, &rxdata, 1);
                    HAL_TIM_Base_Stop_IT(&htim2);
                    __HAL_TIM_SET_COUNTER(&htim2, 0);
                    return;
                }
            }
            rx_check += rxdata;
            rx_index++;
        }
        HAL_UART_Receive_IT(&huart2, &rxdata, 1);
    }
    if(huart->Instance == UART5){
        receiving = 1;
        i++;
        HAL_UART_Receive_IT(&huart5, &rxdata, 1);
    }
};
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    if(hadc->Instance == ADC1) {
        ADC_Busy = 0;
        HAL_TIM_Base_Stop_IT(&htim2);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM2){
        VM_ERR = 1;
        VM_Busy = 0;
        VM_init = 0;
        Scan_Start = 0;
    }
}

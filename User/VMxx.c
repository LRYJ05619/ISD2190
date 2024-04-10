//
// Created by ifasten on 2024/3/19.
//

#include "VMxx.h"

u8 VM_Init_Cmd[7] = {
        0xAA,
        0xBB,
        0x01,
        0x85,
        0x00,
        0x00,
        0xEB
};

u8 VM_Scan_Cmd[5] = {
        0xAA,
        0xAA,
        0x01,
        0x73,
        0xC8,
};

void Init_VM(UART_HandleTypeDef huart){
    HAL_UART_Transmit(&huart, VM_Init_Cmd, 7, 0xFFFF);
}

void Scan_VM(UART_HandleTypeDef huart){
    HAL_UART_Transmit(&huart, VM_Scan_Cmd, 5, 0xFFFF);
}


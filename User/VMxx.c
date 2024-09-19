//
// Created by ifasten on 2024/3/19.
//

#include "VMxx.h"

u8 VM_Init_Cmd[8] = {
        0x01,
        0x06,
        0x00,
        0x05,
        0x00,
        0x00,
        0x99,
        0xCB
};

u8 VM_Scan_Cmd[8] = {
        0x01,
        0x06,
        0x00,
        0x03,
        0x00,
        0x33,
        0x39,
        0xDF
};

u8 VM_Verify_Cmd[8] = {
        0x01,
        0x03,
        0x00,
        0x20,
        0x00,
        0x01,
        0x85,
        0xC0
};

u8 VM_Read_Cmd[8] = {
        0x01,
        0x03,
        0x00,
        0x33,
        0x00,
        0x08,
        0xB4,
        0x03
};

u8 VM_Clear_Cmd[8] = {
        0x01,
        0x06,
        0x00,
        0x20,
        0x00,
        0x00,
        0x88,
        0x00
};

void Init_VM(UART_HandleTypeDef huart) {
    HAL_UART_Transmit(&huart, VM_Init_Cmd, 8, 0xFFFF);
}

void Scan_VM(UART_HandleTypeDef huart) {
    HAL_UART_Transmit(&huart, VM_Scan_Cmd, 8, 0xFFFF);
}

void Read_VM(UART_HandleTypeDef huart) {
    HAL_UART_Transmit(&huart, VM_Read_Cmd, 8, 0xFFFF);
}

void Verify_VM(UART_HandleTypeDef huart) {
    HAL_UART_Transmit(&huart, VM_Verify_Cmd, 8, 0xFFFF);
}

void Clear_VM(UART_HandleTypeDef huart) {
    HAL_UART_Transmit(&huart, VM_Clear_Cmd, 8, 0xFFFF);
}
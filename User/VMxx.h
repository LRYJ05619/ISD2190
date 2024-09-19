//
// Created by ifasten on 2024/3/19.
//

#ifndef ISD2190_VMXX_H
#define ISD2190_VMXX_H

#include "main.h"

void Init_VM(UART_HandleTypeDef huart);
void Scan_VM(UART_HandleTypeDef huart);
void Read_VM(UART_HandleTypeDef huart);
void Verify_VM(UART_HandleTypeDef huart);
void Clear_VM(UART_HandleTypeDef huart);
#endif //ISD2190_VMXX_H

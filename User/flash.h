//
// Created by 15569 on 2024/4/11.
//

#ifndef ISD2190_FLASH_H
#define ISD2190_FLASH_H
#include "main.h"

HAL_StatusTypeDef Flash_Write(uint32_t address, uint8_t *data, uint16_t len);
void Flash_Read(uint32_t address, uint8_t *data, uint16_t len);
#endif //ISD2190_FLASH_H

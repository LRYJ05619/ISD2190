//
// Created by 15569 on 2024/4/11.
//

#ifndef ISD2190_FLASH_H
#define ISD2190_FLASH_H
#include "main.h"

HAL_StatusTypeDef Flash_Write();
void Flash_Read();
HAL_StatusTypeDef Flash_Erase();

HAL_StatusTypeDef Config_Write();
void Config_Read();
HAL_StatusTypeDef Config_Erase();

#endif //ISD2190_FLASH_H

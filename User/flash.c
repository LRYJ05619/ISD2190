//
// Created by 15569 on 2024/4/11.
//

#include "flash.h"


extern Sensor_Info Sensor[16];

//384K flash最后一页
#define SENSOR_INFO_FLASH_ADDRESS (0x0805F800)

HAL_StatusTypeDef Flash_Write(uint32_t address, uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status;
    uint32_t i;

    // 解锁 Flash
    HAL_FLASH_Unlock();

    // 擦除 Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = SENSOR_INFO_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 1; // 根据需要存储的数据量调整擦除的页数
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status == HAL_OK) {
        for (i = 0; i < len; i += 4) {
            // 以半字为单位写入数据
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, *((uint16_t*)(data + i))) != HAL_OK) {
                break; // 如果写入失败，跳出循环
            }
        }
    }

    // 锁定 Flash
    HAL_FLASH_Lock();

    return status;
}

void Flash_Read(uint32_t address, uint8_t *data, uint16_t len) {
    uint32_t i;

    for (i = 0; i < len; i++) {
        data[i] = *(__IO uint8_t*)(address + i);
    }
}


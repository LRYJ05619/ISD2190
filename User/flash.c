//
// Created by 15569 on 2024/4/11.
//

#include <string.h>
#include "flash.h"

//384K flash最后一页
#define SensorInfo_FLASH_ADDRESS (0x0805F800)

HAL_StatusTypeDef Flash_Write(uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status;
    uint32_t typeProgram = FLASH_TYPEPROGRAM_WORD;  // 改为字编程
    uint32_t FlashAddress = SensorInfo_FLASH_ADDRESS;

    // 解锁 Flash
    HAL_FLASH_Unlock();

    // 擦除 Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = SensorInfo_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 1; // 根据需要存储的数据量调整擦除的页数
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status != HAL_OK) {
        return status;
    }

    // 写入数据
    for (uint16_t i = 0; i < len; i += 4) {
        uint32_t data32 = *((uint32_t *)(data + i));
        if (HAL_FLASH_Program(typeProgram, FlashAddress, data32) == HAL_OK) {
            FlashAddress += 4; // 移动到下一个字
        } else {
            // 错误处理
            status = HAL_FLASH_GetError();
            HAL_FLASH_Lock();
            return status;
        }
    }

    // 锁定 Flash
    HAL_FLASH_Lock();

    return status;
}

void Flash_Read(uint8_t *data, uint16_t len) {
    memcpy(data, (uint8_t *)SensorInfo_FLASH_ADDRESS, len);
}


//
// Created by 15569 on 2024/4/11.
//

#include <string.h>
#include "flash.h"

//384K flash���һҳ
#define SensorInfo_FLASH_ADDRESS (0x0805F800)

HAL_StatusTypeDef Flash_Write(uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status;
    uint32_t typeProgram = FLASH_TYPEPROGRAM_WORD;  // ��Ϊ�ֱ��
    uint32_t FlashAddress = SensorInfo_FLASH_ADDRESS;

    // ���� Flash
    HAL_FLASH_Unlock();

    // ���� Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = SensorInfo_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 1; // ������Ҫ�洢������������������ҳ��
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status != HAL_OK) {
        return status;
    }

    // д������
    for (uint16_t i = 0; i < len; i += 4) {
        uint32_t data32 = *((uint32_t *)(data + i));
        if (HAL_FLASH_Program(typeProgram, FlashAddress, data32) == HAL_OK) {
            FlashAddress += 4; // �ƶ�����һ����
        } else {
            // ������
            status = HAL_FLASH_GetError();
            HAL_FLASH_Lock();
            return status;
        }
    }

    // ���� Flash
    HAL_FLASH_Lock();

    return status;
}

void Flash_Read(uint8_t *data, uint16_t len) {
    memcpy(data, (uint8_t *)SensorInfo_FLASH_ADDRESS, len);
}


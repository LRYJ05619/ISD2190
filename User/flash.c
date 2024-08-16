//
// Created by 15569 on 2024/4/11.
//

#include "flash.h"

//384K flash 256byte a page
#define SensorInfo_FLASH_ADDRESS (0x0805F000)
extern SensorInfo Sensor[16];
uint32_t data;
HAL_StatusTypeDef Flash_Write() {
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
    EraseInitStruct.NbPages = 5; // ������Ҫ�洢������������������ҳ��
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status != HAL_OK) {
        return status;
    }

    for (int i = 0; i < 16; i++) {
        // ����Sensor�����ÿ��Ԫ��
        for (int j = 0; j < sizeof(SensorInfo); j += 4) {
            data = *((__IO uint32_t*)(&Sensor[i].sensor_type + j));
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FlashAddress, data) == HAL_OK) {
                FlashAddress += 4;
            } else {
                // ������
                status = HAL_FLASH_GetError();
                HAL_FLASH_Lock();
                return status;
            }
        }
    }

    // ���� Flash
    HAL_FLASH_Lock();

    return status;
}

void Flash_Read() {
    uint32_t FlashAddress = SensorInfo_FLASH_ADDRESS;
    for (int i = 0; i < 16; i++) {
        for(int j = 0;  j < sizeof(SensorInfo); j += 4)
        {
            *((__IO uint32_t*)(&Sensor[i].sensor_type + j)) = *(__IO uint32_t*)FlashAddress;//ע�⸳ֵ�����,����Ҫ�ýṹ���һ����Ա�ĵ�ַ��ƫ��,˫��ƫ������8
            FlashAddress += 4;
        }
    }
}

HAL_StatusTypeDef Flash_Erase(){
    HAL_StatusTypeDef status;
    // ���� Flash
    HAL_FLASH_Unlock();

    // ���� Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = SensorInfo_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 5; // ������Ҫ�洢������������������ҳ��
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
    // ���� Flash
    HAL_FLASH_Lock();

    return status;
}


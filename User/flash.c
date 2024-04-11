//
// Created by 15569 on 2024/4/11.
//

#include "flash.h"


extern Sensor_Info Sensor[16];

//384K flash���һҳ
#define SENSOR_INFO_FLASH_ADDRESS (0x0805F800)

HAL_StatusTypeDef Flash_Write(uint32_t address, uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status;
    uint32_t i;

    // ���� Flash
    HAL_FLASH_Unlock();

    // ���� Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = SENSOR_INFO_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 1; // ������Ҫ�洢������������������ҳ��
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status == HAL_OK) {
        for (i = 0; i < len; i += 4) {
            // �԰���Ϊ��λд������
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, *((uint16_t*)(data + i))) != HAL_OK) {
                break; // ���д��ʧ�ܣ�����ѭ��
            }
        }
    }

    // ���� Flash
    HAL_FLASH_Lock();

    return status;
}

void Flash_Read(uint32_t address, uint8_t *data, uint16_t len) {
    uint32_t i;

    for (i = 0; i < len; i++) {
        data[i] = *(__IO uint8_t*)(address + i);
    }
}


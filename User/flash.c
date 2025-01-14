//
// Created by 15569 on 2024/4/11.
//

#include "flash.h"

<<<<<<< Updated upstream
//384K flash 256bit a page
#define SensorInfo_FLASH_ADDRESS (0x0805F000)
=======
//384K flash 256byte a page
//Senser起始地址
#define SensorInfo_FLASH_ADDRESS (0x0805D800)
//ip端口号，id起始地址
#define ConfigInfo_FLASH_ADDRESS (0x0805DE00)

>>>>>>> Stashed changes
extern SensorInfo Sensor[16];
uint32_t data;

extern ConfigInfo Config;

HAL_StatusTypeDef Flash_Write() {
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
<<<<<<< Updated upstream
    EraseInitStruct.NbPages = 2; // 根据需要存储的数据量调整擦除的页数
=======
    EraseInitStruct.NbPages = 6; // 根据需要存储的数据量调整擦除的页数
>>>>>>> Stashed changes
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status != HAL_OK) {
        return status;
    }

    for (int i = 0; i < 16; i++) {
        // 遍历Sensor数组的每个元素
        for (int j = 0; j < sizeof(SensorInfo); j += 4) {
            data = *((__IO uint32_t*)(&Sensor[i].sensor_type + j));
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FlashAddress, data) == HAL_OK) {
                FlashAddress += 4;
            } else {
                // 错误处理
                status = HAL_FLASH_GetError();
                HAL_FLASH_Lock();
                return status;
            }
        }
    }

    // 锁定 Flash
    HAL_FLASH_Lock();

    return status;
}

void Flash_Read() {
    uint32_t FlashAddress = SensorInfo_FLASH_ADDRESS;
    for (int i = 0; i < 16; i++) {
        for(int j = 0;  j < sizeof(SensorInfo); j += 4)
        {
            *((__IO uint32_t*)(&Sensor[i].sensor_type + j)) = *(__IO uint32_t*)FlashAddress;//注意赋值的左边,必须要用结构体第一个成员的地址来偏移,双字偏移量是8
            FlashAddress += 4;
        }
    }
}

HAL_StatusTypeDef Flash_Erase(){
    HAL_StatusTypeDef status;
    // 解锁 Flash
    HAL_FLASH_Unlock();

    // 擦除 Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = SensorInfo_FLASH_ADDRESS;
<<<<<<< Updated upstream
    EraseInitStruct.NbPages = 2; // 根据需要存储的数据量调整擦除的页数
=======
    EraseInitStruct.NbPages = 6; // 根据需要存储的数据量调整擦除的页数
>>>>>>> Stashed changes
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
    // 锁定 Flash
    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef Config_Write() {
    HAL_StatusTypeDef status;
    uint32_t typeProgram = FLASH_TYPEPROGRAM_WORD;  // 改为字编程
    uint32_t FlashAddress = ConfigInfo_FLASH_ADDRESS;

    // 解锁 Flash
    HAL_FLASH_Unlock();

    // 擦除 Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = ConfigInfo_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 1; // 根据需要存储的数据量调整擦除的页数
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    if (status != HAL_OK) {
        return status;
    }

    for (int j = 0; j < sizeof(ConfigInfo); j += 4) {
        data = *((__IO uint32_t *) (&Config.ip_length + j));
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FlashAddress, data) == HAL_OK) {
            FlashAddress += 4;
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

void Config_Read() {
    uint32_t FlashAddress = ConfigInfo_FLASH_ADDRESS;
    for (int j = 0; j < sizeof(ConfigInfo); j += 4) {
        *((__IO uint32_t *) (&Config.ip_length +
                             j)) = *(__IO uint32_t *) FlashAddress;//注意赋值的左边,必须要用结构体第一个成员的地址来偏移,双字偏移量是8
        FlashAddress += 4;
    }
}

HAL_StatusTypeDef Config_Erase(){
    HAL_StatusTypeDef status;
    // 解锁 Flash
    HAL_FLASH_Unlock();

    // 擦除 Flash
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = ConfigInfo_FLASH_ADDRESS;
    EraseInitStruct.NbPages = 1; // 根据需要存储的数据量调整擦除的页数
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
    // 锁定 Flash
    HAL_FLASH_Lock();

    return status;
}

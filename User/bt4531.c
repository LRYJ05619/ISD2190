//
// Created by ifasten on 2024/4/2.
//

#include "bt4531.h"
#include "usart.h"
#include "flash.h"

u8 tx_buffer[MAX_DATA_LENGTH];
extern SensorInfo Sensor[16];
extern u8 BleBuf[MAX_DATA_LENGTH];

u16 CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN);

//命令处理
void BleProcess(){
    if(0xA0 != BleBuf[0] || 0x01 != BleBuf[2])
        return;
    u16 dataLength = sizeof(BleBuf) / sizeof(BleBuf[0]);
    if(dataLength != BleBuf[1]){
        StatuCallback(BleBuf[2], 0x15);
        return;
    }
    u16 receivedCRC = (BleBuf[dataLength - 2] << 8) | BleBuf[dataLength - 1];
    u16 calculatedCRC = CRC_Check(BleBuf, dataLength - 2); // 不包括校验码本身
    if (receivedCRC != calculatedCRC){
        StatuCallback(BleBuf[3], 0x14);
        return;
    }

    u16 addr;
    u8 master;
    u8 num;

    switch (BleBuf[3]) {
        case 0x60:
            DataSend(BleBuf[4]);
            break;
        case 0x61:
            TotalDataSend();
            break;
        case 0x70:
            ConfigSend(BleBuf[4]);
            break;
        case 0x71:
            TotalConfigSend();
            break;
        case 0x50:
            addr = ((u16)BleBuf[5] << 8) & BleBuf[6];
            for(u8 i =0, flag = 0; i < 16; i++){
                if(BleBuf[4] & (1 << i)){
                    num++;
                    if(flag == 0){
                        flag = 1;
                        master = i;
                        Sensor[i].cancel_addr = addr;
                        Sensor[i].sensor_type = BleBuf[4];
                        Sensor[i].para_size = BleBuf[7];
                        for(int j = 0; j < BleBuf[7]; j++){
                            Sensor[i].para[j] = ((u16)BleBuf[2 * j + 7] << 8) & BleBuf[2 * j + 8];
                        }
                        Sensor[i].status = 0x01;
                    } else{
                        Sensor[i].status = 0x02;
                    }
                }
            }
            Sensor[master].cancel_size = num;
            Flash_Write((uint8_t*)&Sensor, sizeof(Sensor));
            StatuCallback(0x50, 0xA0);
            break;
//        case 0x40:
    }
}

//返回配置信息
void ConfigSend(u8 cancel){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x60;
    tx_buffer[4] = Sensor[cancel].sensor_type;
    tx_buffer[5] = (Sensor[cancel].cancel_addr >> 8) & 0xFF;
    tx_buffer[6] = Sensor[cancel].cancel_addr & 0xFF;

    u8 num = 7;

    for(u8 i = 0; i < Sensor[cancel].cancel_size; i++){
        tx_buffer[num++] = (Sensor[cancel].init_freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[cancel].init_freq[i]) & 0xFF;
        tx_buffer[num++] = Sensor[cancel].init_temp;
    }
    for(u8 j = 0; j < Sensor[cancel].para_size; j++){
        tx_buffer[num++] = (Sensor[cancel].para[j] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[cancel].para[j]) & 0xFF;
    }
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}

void TotalConfigSend(){
    u8 num = 5;
    u8 device = 0;
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x71;

    for(u8 i = 0; i < 16; i++) {
        if (Sensor[i].status != 0x01)
            continue;

        device++;
        tx_buffer[num++] = Sensor[i].sensor_type;
        tx_buffer[num++] = (Sensor[i].cancel_addr >> 8) & 0xFF;
        tx_buffer[num++] = Sensor[i].cancel_addr & 0xFF;

        for(u8 j = 0; j < Sensor[i].cancel_size; j++){
            tx_buffer[num++] = (Sensor[i].init_freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].init_freq[j]) & 0xFF;
            tx_buffer[num++] = Sensor[i].init_temp;
        }
        for(u8 k = 0; k < Sensor[i].para_size; k++){
            tx_buffer[num++] = (Sensor[i].para[k] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].para[k]) & 0xFF;
        }
    }

    tx_buffer[4] = device;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}

//返回数据
void DataSend(u8 cancel){
    u8 num = 6;
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x70;
    tx_buffer[4] = Sensor[cancel].sensor_type;
    tx_buffer[5] = (Sensor[cancel].cancel_addr >> 8) & 0xFF;
    tx_buffer[6] = Sensor[cancel].cancel_addr & 0xFF;

    for(u8 i = 0; i < Sensor[cancel].cancel_size ; i++){
        tx_buffer[num++] = (Sensor[cancel].freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[cancel].freq[i]) & 0xFF;
        tx_buffer[num++] = Sensor[cancel].temp;
        tx_buffer[num++] = ((u16)Sensor[i].Calculate >> 8) & 0xFF ;
        tx_buffer[num++] = (u16)Sensor[i].Calculate & 0xFF;
        tx_buffer[num++] = (u8)((Sensor[i].Calculate - (u16)Sensor[i].Calculate) * 100);
    }
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}
//返回全部数据
void TotalDataSend(){
    u8 num = 5;
    u8 device = 0;
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x71;

    for(u8 i = 0; i < 16; i++) {
        if (Sensor[i].status != 0x01)
            continue;

        device++;
        tx_buffer[num++] = Sensor[i].sensor_type;
        tx_buffer[num++] = (Sensor[i].cancel_addr >> 8) & 0xFF;
        tx_buffer[num++] = Sensor[i].cancel_addr & 0xFF;

        for (u8 j = 0; j < Sensor[i].cancel_size; j++) {
            tx_buffer[num++] = (Sensor[i].freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].freq[j]) & 0xFF;
            tx_buffer[num++] = Sensor[i].temp;
            tx_buffer[num++] = ((u16)Sensor[i].Calculate >> 8) & 0xFF;
            tx_buffer[num++] = (u16)Sensor[i].Calculate & 0xFF;
            tx_buffer[num++] = (u8)((Sensor[i].Calculate - (u16)Sensor[i].Calculate) * 100);
        }
    }

    tx_buffer[4] = device;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}

//状态回调
void StatuCallback(u8 cmd, u8 statu){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[1] = 0x07;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = cmd;
    tx_buffer[4] = statu;

    crc = CRC_Check(tx_buffer, 5);

    tx_buffer[5] = (crc >> 8) & 0xFF;
    tx_buffer[6] = crc & 0xFF;

    HAL_UART_Transmit(&huart5, tx_buffer, 7, HAL_MAX_DELAY);
}

u16 CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN) {
    u16 CRC_Value = 0;
    u8 i = 0;
    u8 j = 0;

    CRC_Value = 0xffff;
    for (i = 0; i < LEN; i++)  //LEN为数组长度
    {
        CRC_Value ^= *(CRC_Ptr + i);
        for (j = 0; j < 8; j++) {
            if (CRC_Value & 0x00001)
                CRC_Value = (CRC_Value >> 1) ^ 0xA001;
            else
                CRC_Value = (CRC_Value >> 1);
        }
    }
    CRC_Value = ((CRC_Value >> 8) + (CRC_Value << 8)); //交换高低字节

    return CRC_Value;
}


//
// Created by ifasten on 2024/4/2.
//

#include <string.h>
#include "bt4531.h"
#include "usart.h"
#include "flash.h"
#include "CollectData.h"

u8 tx_buffer[MAX_DATA_LENGTH];
extern SensorInfo Sensor[16];
extern u8 BleBuf[MAX_DATA_LENGTH];
extern u8 Scan_Start;
extern u8 Cmd;
extern u8 ble_len;

u16 CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN);

//�����
void BleProcess(){
    if(0xA0 != BleBuf[0] || 0x01 != BleBuf[2]){
        ble_len = 0;
        return;
    }
    if(ble_len != BleBuf[1]){
        StatuCallback(BleBuf[2], 0x15);
        ble_len = 0;
        return;
    }


    u16 receivedCRC = (BleBuf[ble_len - 2] << 8) | BleBuf[ble_len - 1];
    u16 calculatedCRC = CRC_Check(BleBuf, ble_len - 2); // ������У���뱾��
    if (receivedCRC != calculatedCRC){
        StatuCallback(BleBuf[3], 0x14);
        return;
    }

    Cmd = BleBuf[3];

    ble_len = 0;

    u16 addr;
    u8 master;
    u8 num;

    switch (BleBuf[3]) {
        case 0x70:
            Scan_Start = 0x01;
            break;
        case 0x71:
            Scan_Start = 0x03;
            break;
        case 0x60:
            ConfigSend(BleBuf[4]);
            break;
        case 0x61:
            TotalConfigSend();
            break;
        case 0x50:
            addr = ((u16)BleBuf[10] << 8) | BleBuf[11];
            for(u8 i =0, flag = 0; i < 16; i++){
                if(addr & (1 << i)){
                    num++;
                    if(flag == 0){
                        flag = 1;
                        master = i;
                        for (u8 k = 0; k < 5; k++) {
                            Sensor[i].serial[k] = BleBuf[k + 4];
                        }
                        Sensor[i].channel_addr = addr;
                        Sensor[i].sensor_type = BleBuf[9];
                        Sensor[i].para_size = BleBuf[12];
                        for(int j = 0; j < BleBuf[12]; j++){
                            Sensor[i].para[j] = (int16_t)BleBuf[2 * j + 13] << 16 | BleBuf[2 * j + 14] << 8 | BleBuf[2 * j + 15];
                        }
                        Sensor[i].status = 0x01;
                    } else{
                        Sensor[i].status = 0x02;
                    }
                }
            }
            Sensor[master].channel_size = num;

            Flash_Write();
            StatuCallback(0x50, 0xA0);
            break;
        case 0x40:
            Scan_Start = 0x05;
            break;
    }
}

//����������Ϣ
void ConfigSend(u8 channel){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x60;

    for (u8 k = 0; k < 5; k++) {
        tx_buffer[k + 4] = Sensor[channel].serial[k];
    }

    tx_buffer[9] = Sensor[channel].sensor_type;
    tx_buffer[10] = (Sensor[channel].channel_addr >> 8) & 0xFF;
    tx_buffer[11] = Sensor[channel].channel_addr & 0xFF;

    u8 num = 12;

    for(u8 i = 0; i < Sensor[channel].channel_size; i++){
        tx_buffer[num++] = (Sensor[channel].init_freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[channel].init_freq[i]) & 0xFF;
    }
    tx_buffer[num++] = Sensor[channel].init_temp;
    tx_buffer[num++] = Sensor[channel].para_size;
    for(u8 j = 0; j < Sensor[channel].para_size; j++){
        tx_buffer[num++] = (Sensor[channel].para[j] >> 16) & 0xFF;
        tx_buffer[num++] = (Sensor[channel].para[j] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[channel].para[j]) & 0xFF;
    }
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}
//����ȫ������
void TotalConfigSend(){
    u8 device = 0;
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x61;
    u16 num = 5;

    for(u8 i = 0; i < 16; i++) {
        if (Sensor[i].status != 0x01)
            continue;

        device++;
        for (u8 l = 0; l < 5; l++) {
            tx_buffer[num++] = Sensor[i].serial[l];
        }
        tx_buffer[num++] = Sensor[i].sensor_type;
        tx_buffer[num++] = (Sensor[i].channel_addr >> 8) & 0xFF;
        tx_buffer[num++] = Sensor[i].channel_addr & 0xFF;

        for(u8 j = 0; j < Sensor[i].channel_size; j++){
            tx_buffer[num++] = (Sensor[i].init_freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].init_freq[j]) & 0xFF;
        }
        tx_buffer[num++] = Sensor[i].init_temp;
        tx_buffer[num++] = Sensor[i].para_size;
        for(u8 k = 0; k < Sensor[i].para_size; k++){
            tx_buffer[num++] = (Sensor[i].para[k] >> 16) & 0xFF;
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

//��������
void DataSend(u8 channel){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x70;

    for (u8 k = 0; k < 5; k++) {
        tx_buffer[k + 4] = Sensor[channel].serial[k];
    }

    tx_buffer[9] = Sensor[channel].sensor_type;
    tx_buffer[10] = Sensor[channel].channel_size;

    u8 num = 11;

    for(u8 i = 0; i < Sensor[channel].channel_size ; i++){
        tx_buffer[num++] = (Sensor[channel].freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[channel].freq[i]) & 0xFF;
    }
    tx_buffer[num++] = ((u16)Sensor[channel].Calculate >> 16 ) * 100 & 0xFF;
    tx_buffer[num++] = ((u16)Sensor[channel].Calculate >> 8 ) * 100 & 0xFF;
    tx_buffer[num++] = ((u16)Sensor[channel].Calculate) * 100 & 0xFF;

    tx_buffer[num++] = Sensor[channel].temp;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}
//����ȫ������
void TotalDataSend(){
    u8 device = 0;
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x71;
    u8 num = 5;

    for(u8 i = 0; i < 16; i++) {
        if (Sensor[i].status != 0x01)
            continue;

        device++;
        for (u8 l = 0; l < 5; l++) {
            tx_buffer[num++] = Sensor[i].serial[l];
        }
        tx_buffer[num++] = Sensor[i].sensor_type;
        tx_buffer[num++] = Sensor[i].channel_size;

        for (u8 j = 0; j < Sensor[i].channel_size; j++) {
            tx_buffer[num++] = (Sensor[i].freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].freq[j]) & 0xFF;
        }
        tx_buffer[num++] = ((u16)Sensor[i].Calculate >> 16 ) * 100 & 0xFF;
        tx_buffer[num++] = ((u16)Sensor[i].Calculate >> 8 ) * 100 & 0xFF;
        tx_buffer[num++] = ((u16)Sensor[i].Calculate) * 100 & 0xFF;
        tx_buffer[num++] = Sensor[i].temp;
    }

    tx_buffer[4] = device;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}

//״̬�ص�
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

void ConfigInit(){
    memcpy(Sensor[BleBuf[4] - 1].init_freq, Sensor[BleBuf[4] - 1].freq, Sensor[BleBuf[4] - 1].channel_size * 2);
    Sensor[BleBuf[4] - 1].init_temp = Sensor[BleBuf[4] - 1].temp;

    Flash_Write();
    StatuCallback(0x40, 0xA0);
}

u16 CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN) {
    u16 CRC_Value = 0;
    u8 i = 0;
    u8 j = 0;

    CRC_Value = 0xffff;
    for (i = 0; i < LEN; i++)  //LENΪ���鳤��
    {
        CRC_Value ^= *(CRC_Ptr + i);
        for (j = 0; j < 8; j++) {
            if (CRC_Value & 0x00001)
                CRC_Value = (CRC_Value >> 1) ^ 0xA001;
            else
                CRC_Value = (CRC_Value >> 1);
        }
    }

    return CRC_Value;
}


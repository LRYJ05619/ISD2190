//
// Created by ifasten on 2024/4/2.
//

#include "bt4531.h"
#include "main.h"
#include "usart.h"

u8 tx_buffer[MAX_DATA_LENGTH];
extern Sensor_Info Sensor[16];

u16 CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN);

//����������Ϣ
void ConfigSend(u8 device_cancel){

}

void TotalConfigSend(){

}

//��������
void DataSend(u8 cancel){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x70;
    tx_buffer[4] = Sensor[cancel].sensor_type;
    tx_buffer[5] = Sensor[cancel].cancel_addr;

    u8 num = 6;

    for(u8 i = 0; i < Sensor[cancel].size ; i++){
        tx_buffer[num++] = (Sensor[cancel].freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[cancel].freq[i]) & 0xFF;
        tx_buffer[num++] = Sensor[cancel].temp;
    }
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit(&huart5, tx_buffer,  num, HAL_MAX_DELAY);
}
//����ȫ������
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
        tx_buffer[num++] = Sensor[i].cancel_addr;
        for (u8 j = 0; j < Sensor[i].size; j++) {
            tx_buffer[num++] = (Sensor[i].freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].freq[j]) & 0xFF;
            tx_buffer[num++] = Sensor[i].temp;
            tx_buffer[num++] = ((u16)Sensor[i].Calculate >> 8) & 0xFF ;
            tx_buffer[num++] = (u16)Sensor[i].Calculate >> 8;
            
        }
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
    CRC_Value = ((CRC_Value >> 8) + (CRC_Value << 8)); //�����ߵ��ֽ�

    return CRC_Value;
}


//
// Created by ifasten on 2024/4/2.
//

#include <string.h>
#include "bt4531.h"
#include "usart.h"
#include "flash.h"
#include "CollectData.h"

u8 tx_buffer[512];
extern SensorInfo Sensor[16];
extern u8 BleBuf[BLE_CONFIG_BUFFER_SIZE];
extern volatile u8 Cmd;
extern u8 ble_len;

extern ConfigInfo Config;

extern u8 Scan_Start;

u16 CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN);

//命令处理
void BleProcess(){
    if(0xA0 != BleBuf[0] || 0x01 != BleBuf[2]){
        return;
    }
    if(ble_len != BleBuf[1]){
        StatuCallback(BleBuf[2], 0x15);
        return;
    }

    u16 receivedCRC = (BleBuf[ble_len - 2] << 8) | BleBuf[ble_len - 1];
    u16 calculatedCRC = CRC_Check(BleBuf, ble_len - 2); // 不包括校验码本身
    if (receivedCRC != calculatedCRC){
        StatuCallback(BleBuf[3], 0x14);
        return;
    }

    Cmd = BleBuf[3];

    ble_len = 0;

    switch (Cmd) {
        case 0x70:
            Scan_Start = 1;
            break;
        case 0x71:
            Scan_Start = 1;
            break;
        case 0x60:
            ConfigSend(BleBuf[4]);
            break;
        case 0x61:
            TotalConfigSend();
            break;
        case 0x50:
            SenserConfig();
            break;
        case 0x40:
            Scan_Start = 1;
            break;
        case 0x30:
            IpConfigSend();
            break;
        case 0x31:
            IpConfig();
            break;
        case 0x32:
            IdConfig();
            break;
        case 0x20:
            RewriteConfig();
            break;
    }
}

//配置传感器
void SenserConfig(){
    u16 addr;
    u8 master;
    u8 num;

    addr = ((u16)BleBuf[9] << 8) | BleBuf[10];
    for(u8 i =0, flag = 0; i < 16; i++){
        if(addr & (1 << i)){
            num++;
            if(flag == 0){
                flag = 1;
                master = i;
                for (u8 k = 0; k < 4; k++) {
                    Sensor[i].sensor_model[k] = BleBuf[k + 5];
                }
                Sensor[i].channel_addr = addr;
                Sensor[i].sensor_type = BleBuf[4];
                Sensor[i].para_size = BleBuf[11];
                for(int j = 0; j < BleBuf[11]; j++){
                    uint8_t *valuePtr = (uint8_t *)&Sensor[i].para[j];
                    for (int k = 0; k < 8; k++) {
                        valuePtr[k] = BleBuf[8 * j + 12 + k];
                    }
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
}

//返回配置信息
void ConfigSend(u8 channel){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x60;

    for (u8 k = 0; k < 4; k++) {
        tx_buffer[k + 5] = Sensor[channel].sensor_model[k];
    }

    tx_buffer[4] = Sensor[channel].sensor_type;
    tx_buffer[9] = (Sensor[channel].channel_addr >> 8) & 0xFF;
    tx_buffer[10] = Sensor[channel].channel_addr & 0xFF;

    u16 num = 11;

    for(u8 i = 0; i < Sensor[channel].channel_size; i++){
        tx_buffer[num++] = (Sensor[channel].init_freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[channel].init_freq[i]) & 0xFF;
    }
    tx_buffer[num++] = Sensor[channel].init_temp;
    tx_buffer[num++] = Sensor[channel].para_size;
    for(u8 j = 0; j < Sensor[channel].para_size; j++){
        uint8_t *doublePtr = (uint8_t *)&Sensor[channel].para[j];
        memcpy(&tx_buffer[num], doublePtr, sizeof(double));
        num+=sizeof(double);
    }
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit_DMA(&huart5, tx_buffer,  num);
}
//返回全部配置
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

        tx_buffer[num++] = Sensor[i].sensor_type;
        for (u8 l = 0; l < 4; l++) {
            tx_buffer[num++] = Sensor[i].sensor_model[l];
        }
        tx_buffer[num++] = (Sensor[i].channel_addr >> 8) & 0xFF;
        tx_buffer[num++] = Sensor[i].channel_addr & 0xFF;

        for(u8 j = 0; j < Sensor[i].channel_size; j++){
            tx_buffer[num++] = (Sensor[i].init_freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].init_freq[j]) & 0xFF;
        }
        tx_buffer[num++] = Sensor[i].init_temp;
        tx_buffer[num++] = Sensor[i].para_size;
        for(u8 k = 0; k < Sensor[i].para_size; k++){
            u8 *doublePtr = (uint8_t *)&Sensor[i].para[k];
            memcpy(&tx_buffer[num], doublePtr, sizeof(double));
            num+=sizeof(double);
        }
    }

    tx_buffer[4] = device;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;

    HAL_UART_Transmit_DMA (&huart5, tx_buffer,  num);
}

//返回数据
void DataSend(u8 channel){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x70;

    for (u8 k = 0; k < 4; k++) {
        tx_buffer[k + 5] = Sensor[channel].sensor_model[k];
    }

    tx_buffer[4] = Sensor[channel].sensor_type;
    tx_buffer[9] = Sensor[channel].channel_size;

    u16 num = 10;

    for(u8 i = 0; i < Sensor[channel].channel_size ; i++){
        tx_buffer[num++] = (Sensor[channel].freq[i] >> 8) & 0xFF;
        tx_buffer[num++] = (Sensor[channel].freq[i]) & 0xFF;
    }
    uint8_t* p = (uint8_t*)&Sensor[channel].Calculate;
    tx_buffer[num++] = *p;
    tx_buffer[num++] = *(p+1);
    tx_buffer[num++] = *(p+2);
    tx_buffer[num++] = *(p+3);

    tx_buffer[num++] = Sensor[channel].temp;
    tx_buffer[num++] = Sensor[channel].freq_status;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit_DMA(&huart5, tx_buffer,  num);
}
//返回全部数据
void TotalDataSend(){
    u8 device = 0;
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x71;
    u16 num = 5;

    for(u8 i = 0; i < 16; i++) {
        if (Sensor[i].status != 0x01)
            continue;

        device++;
        tx_buffer[num++] = Sensor[i].sensor_type;
        for (u8 l = 0; l < 4; l++) {
            tx_buffer[num++] = Sensor[i].sensor_model[l];
        }

        tx_buffer[num++] = Sensor[i].channel_size;

        for (u8 j = 0; j < Sensor[i].channel_size; j++) {
            tx_buffer[num++] = (Sensor[i].freq[j] >> 8) & 0xFF;
            tx_buffer[num++] = (Sensor[i].freq[j]) & 0xFF;
        }
        uint8_t* p = (uint8_t*)&Sensor[i].Calculate;
        tx_buffer[num++] = *p;
        tx_buffer[num++] = *(p+1);
        tx_buffer[num++] = *(p+2);
        tx_buffer[num++] = *(p+3);

        tx_buffer[num++] = Sensor[i].temp;

        tx_buffer[num++] = Sensor[i].freq_status;
    }

    tx_buffer[4] = device;
    tx_buffer[1] = num + 2;
    crc = CRC_Check(tx_buffer, num);
    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;
    HAL_UART_Transmit_DMA(&huart5, tx_buffer,  num);
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

    HAL_UART_Transmit_DMA(&huart5, tx_buffer, 7);
}

void ConfigInit(){
    memcpy(Sensor[BleBuf[4] - 1].init_freq, Sensor[BleBuf[4] - 1].freq, Sensor[BleBuf[4] - 1].channel_size * 2);
    Sensor[BleBuf[4] - 1].init_temp = Sensor[BleBuf[4] - 1].temp;

    Flash_Write();
    StatuCallback(0x40, 0xA0);
}

//配置Ip端口号
void IpConfig(){
    Config.ip_length = BleBuf[1] - 6;
    for (int i = 0; i < Config.ip_length; i++) {
        Config.ip[i] = BleBuf[i + 4];
    }
    Config_Write();
    StatuCallback(0x31, 0xA0);
}
//配置Id
void IdConfig(){
    for (int i = 0; i < 19; i++) {
        Config.id[i] = BleBuf[i + 4];
    }
    Config_Write();

    //25.6.4 新增即时修改蓝牙名称
    u8 ble_name[40];
    strncpy( ble_name, "TTM:REN-",strlen( "TTM:REN-" ));
    strncat( ble_name, Config.id, 19);
    strncat( ble_name, "\r\n\0",strlen( "\r\n\0" ));
    HAL_UART_Transmit_DMA(&huart5, ble_name, strlen(ble_name));

    StatuCallback(0x32, 0xA0);
}
//返回配置信息
void IpConfigSend(){
    u16 crc;

    tx_buffer[0] = 0xA0;
    tx_buffer[2] = 0x01;
    tx_buffer[3] = 0x30;

    u8 num = 4;

    for (int i = 0; i < 19; i++) {
        tx_buffer[num++] = Config.id[i];
    }

    for (int i = 0; i < Config.ip_length; i++) {
        tx_buffer[num++] = Config.ip[i];
    }

    //25.6.4 修正crc校验错误
    tx_buffer[1] = num + 2;

    crc = CRC_Check(tx_buffer, tx_buffer[1]);

    tx_buffer[num++] = (crc >> 8) & 0xFF;
    tx_buffer[num++] = crc & 0xFF;

    HAL_UART_Transmit_DMA(&huart5, tx_buffer, num);
}
//重写配置
void RewriteConfig() {
    u8 device_num = BleBuf[4]; // 读取有效设备的数量
    u8 pos = 5; // 当前位置，开始从rx_buffer[5]解析

    for (u8 i = 0; i < device_num; i++) {
        u8 type;
        u8 model[4];
        u16 channel_addr;
        u16 channel_size = 0;
        u8 channel;
        u8 flag = 0;

        // 开始解读每个字段
        type = BleBuf[pos++];

        // 读取传感器型号
        for (u8 l = 0; l < 4; l++) {
            model[l] = BleBuf[pos++];
        }

        // 读取通道地址
        channel_addr = ((u16)BleBuf[pos++] << 8);
        channel_addr |= BleBuf[pos++];

        for (u8 ch = 0; ch < 16; ch++) {
            if ((channel_addr >> ch) & 0x01) {  // 依次判断每个通道位
                channel_size++;
                if (flag == 0) {
                    // 找到主通道
                    flag = 1;
                    channel = ch;
                    Sensor[channel].status = 0x01;  // 设置为主通道
                } else {
                    // 其他为从通道
                    Sensor[ch].status = 0x02;  // 设置为从通道
                }
            }
        }
        Sensor[channel].sensor_type = type;
        Sensor[channel].channel_size = channel_size;
        Sensor[channel].channel_addr = channel_addr;
        for (u8 m = 0; m < 4; m++) {
            Sensor[channel].sensor_model[m] = model[m];
        }

        // 读取初始频率（数量为 channel_size）
        for (u8 j = 0; j < channel_size; j++) {
            Sensor[channel].init_freq[j] = ((u16)BleBuf[pos++] << 8);
            Sensor[channel].init_freq[j] |= BleBuf[pos++];
        }

        // 读取初始温度
        Sensor[channel].init_temp = (int8_t)BleBuf[pos++];

        // 读取参数数量
        Sensor[channel].para_size = BleBuf[pos++];

        // 读取参数（每个参数为 double 类型）
        for (u8 k = 0; k < Sensor[channel].para_size; k++) {
            memcpy(&Sensor[channel].para[k], &BleBuf[pos], sizeof(double));
            pos += sizeof(double);
        }
    }
    Flash_Write();
    StatuCallback(0x20, 0xA0);
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

    return CRC_Value;
}


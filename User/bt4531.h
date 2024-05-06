//
// Created by ifasten on 2024/4/2.
//

#ifndef ISD2190_BT4531_H
#define ISD2190_BT4531_H

#include "main.h"

void BleProcess();
void ConfigSend(u8 channel);
void TotalConfigSend();
void DataSend(u8 channel);
void TotalDataSend();
void StatuCallback(u8 cmd, u8 statu);
void ConfigInit();

#endif //ISD2190_BT4531_H

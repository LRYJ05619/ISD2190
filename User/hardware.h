//
// Created by ifasten on 2024/3/14.
//

#ifndef ISD2190_HARDWARE_H
#define ISD2190_HARDWARE_H

#include "main.h"
#include <math.h>

float BMYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp);
float MRYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp);
float GJJ_YL(u16 init_freq, u8 init_temp, u16 freq, u8 temp, double K);
float TYLH_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, double K);
float SYJ_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, double A, double B, double C, double b);
float MSJ_YL_KN(u16* init_freq, u8 init_temp, u16* freq, u8 temp, u8 size, double a, double b);
#endif //ISD2190_HARDWARE_H

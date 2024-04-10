//
// Created by ifasten on 2024/3/14.
//

#ifndef ISD2190_HARDWARE_H
#define ISD2190_HARDWARE_H

#include "main.h"
#include <math.h>

double BMYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp);
double MRYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp);
double GJJ_YL(u16 init_freq, u8 init_temp, u16 freq, u8 temp, u16 K);
double TYLH_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, u16 K);
double SYJ_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, u16 A, u16 B, u16 C, u16 b);
double MSJ_YL_KN(u16* init_freq, u8 init_temp, u16* freq, u8 temp, u8 size, u16 a, u16 b);
#endif //ISD2190_HARDWARE_H

//
// Created by ifasten on 2024/3/14.
//

#ifndef ISD2190_HARDWARE_H
#define ISD2190_HARDWARE_H

#include "main.h"
#include <math.h>

float BMYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp);
float MRYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp);
float GJJ_YL(u16 init_freq, u8 init_temp, u16 freq, u8 temp, int32_t K);
float TYLH_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, int32_t K);
float SYJ_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, int32_t A, int32_t B, int32_t C, int32_t b);
float MSJ_YL_KN(u16* init_freq, u8 init_temp, u16* freq, u8 temp, u8 size, int32_t a, int32_t b);
#endif //ISD2190_HARDWARE_H

//
// Created by ifasten on 2024/3/14.
//

#include "hardware.h"


//表面应变计
double  BMYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp){
    double  YB_Change;
    double  K = 0.0035708;
    double  F = 2.2;
    double  init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    double  YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = K * (YB - init_YB) + F * (temp - init_temp);
    return YB_Change;
}
//埋入式应变计
double MRYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp){
    double YB_Change;
    double K = 0.0031559;
    double F = 2.2;
    double init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    double YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = K * (YB - init_YB) + F * (temp - init_temp);
    return YB_Change;
}
//钢筋计
double GJJ_YL(u16 init_freq, u8 init_temp, u16 freq, u8 temp, int32_t K){
    double YB_Change;
    double K0 = 0.00071186;
    double F = 2.2;
    double init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    double YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = (K * 0.0001) * K0 * (YB - init_YB) + F * (temp - init_temp);
    return YB_Change;
}
//土压力盒
double TYLH_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, int32_t K){
    double YB_Change;
    double K0 = 0.0005383;
    double F = 2.2;
    double init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    double YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = (K * 0.0001) * K0 * (YB - init_YB) + F * (temp - init_temp);
    return YB_Change;
}
//渗压计
double SYJ_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, int32_t A, int32_t B, int32_t C, int32_t b){
    double YB_Change;

    double init_YB = (init_freq * 0.1) * (init_freq * 0.1) * 0.001;
    double YB = (freq * 0.1) * (freq * 0.1) * 0.001;

    YB_Change = (A * 0.0001) * (YB - init_YB) * (YB - init_YB) + (B * 0.0001) * (YB - init_YB) + (C * 0.0001) + (b *0.0001) * (temp - init_temp);
    return YB_Change;
}
//锚索计
double MSJ_YL_KN(u16* init_freq, u8 init_temp, u16* freq, u8 temp, u8 size, int32_t a, int32_t b){
    double k;
    double F;
    if(size == 6){
        k = 0.00084107;
        F = -0.7;
    }
    if(size == 4){
        k = 0.00084107;
        F = -0.5;
    }

    if(size == 3){
        k = 0.0005383;
        F = -0.3;
    }

    double YB_Change;

    double init_total = 0;
    for(u8 i = 0; i < size; i++){
        init_total += ((init_freq[i] * 0.1) * (init_freq[i] * 0.1));
    }
    double init_YB = init_total / size;

    double total = 0;
    for(u8 i = 0; i < size; i++){
        total += ((freq[i] * 0.1) * (freq[i] * 0.1));
    }
    double YB = total / size;

    YB_Change = k * (a * 0.0001) * (YB - init_YB) - (b * 0.0001) + F * (temp - init_temp);
    return YB_Change;
}

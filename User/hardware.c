//
// Created by ifasten on 2024/3/14.
//

#include "hardware.h"


//表面应变计
float  BMYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp){
    float  YB_Change;
    float  K = 0.0035708;
    float  F = 2.2;
    float  init_YB = ((init_freq * 0.1) * (init_freq * 0.1));
    float  YB = ((freq * 0.1) * (freq * 0.1));
    YB_Change = (K * (YB - init_YB)) + F * ((temp - init_temp));
    return YB_Change;
}
//埋入式应变计
float MRYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp){
    float YB_Change;
    float K = 0.0031559;
    float F = 2.2;
    float init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    float YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = (K * (YB - init_YB)) + (F * (temp - init_temp));
    return YB_Change;
}
//钢筋计
float GJJ_YL(u16 init_freq, u8 init_temp, u16 freq, u8 temp, double K){
    float YB_Change;
    float K0 = 0.00071186;
    float F = 2.2;
    float init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    float YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = (K * K0 * (YB - init_YB)) + (F * (temp - init_temp));
    return YB_Change;
}
//土压力盒
float TYLH_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, double K){
    float YB_Change;
    float K0 = 0.0005383;
    float F = 2.2;
    float init_YB = (init_freq * 0.1) * (init_freq * 0.1);
    float YB = (freq * 0.1) * (freq * 0.1);
    YB_Change = (K * K0 * (YB - init_YB)) + (F * (temp - init_temp));
    return YB_Change;
}
//渗压计
float SYJ_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, double A, double B, double C, double b){
    float YB_Change;

    float init_YB = (init_freq * 0.1) * (init_freq * 0.1) * 0.001;
    float YB = (freq * 0.1) * (freq * 0.1) * 0.001;

    YB_Change = (A  * (YB - init_YB) * (YB - init_YB)) + (B * (YB - init_YB)) + C + (b * (temp - init_temp));
    return YB_Change;
}
//锚索计
float MSJ_YL_KN(u16* init_freq, u8 init_temp, u16* freq, u8 temp, u8 size, double a, double b){
    float k;
    float F;
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

    float YB_Change;

    float init_total = 0;
    for(u8 i = 0; i < size; i++){
        init_total += ((init_freq[i] * 0.1) * (init_freq[i] * 0.1));
    }
    float init_YB = init_total / size;

    float total = 0;
    for(u8 i = 0; i < size; i++){
        total += ((freq[i] * 0.1) * (freq[i] * 0.1));
    }
    float YB = total / size;

    YB_Change = (k * a * (YB - init_YB)) - b + (F * (temp - init_temp));
    return YB_Change;
}



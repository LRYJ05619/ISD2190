//
// Created by ifasten on 2024/3/14.
//

#include "hardware.h"


//表面应变计
double  BMYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp){
    double  YB_change;
    double  YB_Change;
    double  K = 0.0035708;
    double  F = 2.2;
    double  init_YB = ((double )init_freq * 0.1) * ((double )init_freq * 0.1);
    double  YB = ((double )freq * 0.1) * ((double )freq * 0.1);
    YB_Change = K * (YB - init_YB) + F * (temp - init_temp);
    YB_change = roundf(YB_Change * 100) / 100;
    return YB_change;
}
//埋入式应变计
double MRYBJ_YB(u16 init_freq, u8 init_temp, u16 freq, u8 temp){
    double YB_change;
    double YB_Change;
    double K = 0.0031559;
    double F = 2.2;
    double init_YB = ((double)init_freq * 0.1) * ((double)init_freq * 0.1);
    double YB = ((double)freq * 0.1) * ((double)freq * 0.1);
    YB_Change = K * (YB - init_YB) + F * (temp - init_temp);
    YB_change = roundf((double)YB_Change * 100) / 100;
    return YB_change;
}
//钢筋计
double GJJ_YL(u16 init_freq, u8 init_temp, u16 freq, u8 temp, u16 K){
    double YL;
    double YB_Change;
    double K0 = 0.00071186;
    double F = 2.2;
    double init_YB = ((double)init_freq * 0.1) * ((double)init_freq * 0.1);
    double YB = ((double)freq * 0.1) * ((double)freq * 0.1);
    YB_Change = K * K0 * (YB - init_YB) + F * (temp - init_temp);
    YL = roundf(YB_Change * 100) / 100;
    return YL;
}
//土压力盒
double TYLH_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, u16 K){
    double YL;
    double YB_Change;
    double K0 = 0.0005383;
    double F = 2.2;
    double init_YB = ((double)init_freq * 0.1) * ((double)init_freq * 0.1);
    double YB = ((double)freq * 0.1) * ((double)freq * 0.1);
    YB_Change = K * K0 * (YB - init_YB) + F * (temp - init_temp);
    YL = roundf(YB_Change * 100) / 100;
    return YL;
}
//渗压计
double SYJ_YL_MPa(u16 init_freq, u8 init_temp, u16 freq, u8 temp, u16 A, u16 B, u16 C, u16 b){
    double YL;
    double YB_Change;

    double init_YB = ((double)init_freq * 0.1) * ((double)init_freq * 0.1) * 0.001f;
    double YB = ((double)freq * 0.1) * ((double)freq * 0.1) * 0.001f;

    YB_Change = A * (YB - init_YB) * (YB - init_YB) + B * (YB - init_YB) + C + b * (temp - init_temp);

    YL = roundf(YB_Change * 100) / 100;
    return YL;
}
//锚索计
double MSJ_YL_KN(u16* init_freq, u8 init_temp, u16* freq, u8 temp, u8 size, u16 a, u16 b){
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

    double YL;
    double YB_Change;

    double init_total;
    for(u8 i = 0; i < size; i++){
        init_total += (((double)init_freq[i] * 0.1) * ((double)init_freq[i] * 0.1));
    }
    double init_YB = init_total / size;

    double total;
    for(u8 i = 0; i < size; i++){
        total += (((double)freq[i] * 0.1) * ((double)freq[i] * 0.1));
    }
    double YB = total / size;

    YB_Change = k * a * (YB - init_YB) - b + F * (temp - init_temp);
    YL = roundf(YB_Change * 100) / 100;
    return YL;
}

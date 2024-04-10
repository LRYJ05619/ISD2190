//
// Created by 15569 on 2024/3/14.
//
#include "temp.h"
#include <stdio.h>
#include <stdlib.h>

static const u16 tempRes_buf[81] = {//3K热敏电阻ad值温度对照
        1023,   1006,   984,    962,    940,    917,    895,    872,    849,    827, // -30℃  ···  -21℃
        804,    782,    760,    738,    716,    694,    673,    652,    631,    610, // -20℃  ···  -11℃
        590,    571,    551,    532,    514,    496,    478,    461,    445,    428, // -10℃  ···   -1℃
        413,    397,    383,    368,    354,    341,    328,    316,    304,    292, //   0℃  ···    9℃
        281,    270,    262,    249,    240,    230,    221,    213,    205,    197, //  10℃  ···   19℃
        189,    182,    175,    168,    161,    155,    149,    143,    138,    133, //  20℃  ···   29℃
        128,    123,    118,    113,    109,    105,    101,     97,     93,     90, //  30℃  ···   39℃
         87,     83,     80,     77,     74,     72,     69,     67,     64,     62, //  40℃  ···   49℃
         60                                                                          //  50℃
};

int8_t calcuTem(u16 ad_value) {
    u16 minDiff = 0xFFFF;
    int16_t closestIndex = 0;

    if(ad_value < 1024 && ad_value > 59) {
        for (u8 i = 0; i < 81; i++) {
            int diff = abs(tempRes_buf[i] - ad_value);
            if (diff < minDiff) {
                minDiff = diff;
                closestIndex = i;
            }
        }
    }
    return closestIndex - 30; // Adjust for the temperature offset
}

void calcuTemps(u16* ad_value, int8_t* Temp_value, u8 size){
  for (int i = 0; i < size; i++) {
        Temp_value[i] = calcuTem(ad_value[i]);
  }
}
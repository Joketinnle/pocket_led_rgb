#ifndef __hsv_2_rgb_h
#define __hsv_2_rgb_h
#include "led.h"

typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
}hsv_t;

void hsv2rgb(hsv_t* hsv, led_bright_t* rgb);

#endif

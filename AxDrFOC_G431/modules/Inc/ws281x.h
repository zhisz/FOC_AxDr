#ifndef _WS281X_H
#define _WS281X_H

#include "main.h"

#define BSP_WS2812_TIM_HANDLE htim2
#define BSP_WS2812_TIM_CHANNEL TIM_CHANNEL_1
#define BSP_WS2812_TIM_INIT MX_TIM2_Init

#define BSP_WS2812_PIXEL_NUM 1  //一个像素点，也就是一个rgb灯
#define BSP_WS2812_DATA_NUM (24 * BSP_WS2812_PIXEL_NUM + 300) // Reset 280us / 1.25us = 224
#define WS1 70                                                //逻辑1
#define WS0 26                                                //逻辑0

///RGB颜色配比
/*
天蓝色 25，255，255
诱惑紫 1，0，1
柠檬黄 255, 128, 0
湛蓝绿 0, 255, 128
深绿色 0, 102, 51

*/

void bsp_ws2812_init(void);
void bsp_ws2812_transmit(void);
void bsp_ws2812_off(void);
void bsp_ws2812_set_all(uint32_t color);
void bsp_ws2812_set_pixel(uint16_t n, uint32_t color);
void bsp_ws2812_rainbow(uint8_t period);
void bsp_ws2812_rainbow_cycle(uint8_t loop_period);


///USER FUNCTION
void RGB_SetColor(uint8_t r_ratio,uint8_t g_ratio,uint8_t b_ratio,uint8_t level);
void RGB_RainbowCycle(uint8_t level, uint16_t speed_ms);



#endif

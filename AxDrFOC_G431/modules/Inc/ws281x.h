#ifndef _WS281X_H
#define _WS281X_H

#include "main.h"

#define BSP_WS2812_TIM_HANDLE htim2
#define BSP_WS2812_TIM_CHANNEL TIM_CHANNEL_1
#define BSP_WS2812_TIM_INIT MX_TIM2_Init

#define BSP_WS2812_PIXEL_NUM 1
#define BSP_WS2812_DATA_NUM (24 * BSP_WS2812_PIXEL_NUM + 300) // Reset 280us / 1.25us = 224
#define WS1 70
#define WS0 26

void bsp_ws2812_init(void);
void bsp_ws2812_transmit(void);
void bsp_ws2812_off(void);
void bsp_ws2812_set_all(uint32_t color);
void bsp_ws2812_set_pixel(uint16_t n, uint32_t color);
void bsp_ws2812_rainbow(uint8_t period);
void bsp_ws2812_rainbow_cycle(uint8_t loop_period);
#endif

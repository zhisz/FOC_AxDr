/**
 ******************************************************************************
 * @file           : ws281x.c
 * @brief          : ws281x LED Driver
 ******************************************************************************
 * @attention
 ******************************************************************************
 */

#include "ws281x.h"
#include <string.h>
#include "tim.h"

static uint16_t ws2812_data[BSP_WS2812_DATA_NUM];

void bsp_ws2812_init(void)
{
	if (HAL_TIM_Base_GetState(&BSP_WS2812_TIM_HANDLE) != HAL_TIM_STATE_READY)
	{
		BSP_WS2812_TIM_INIT();
	}

	HAL_TIM_Base_Start(&BSP_WS2812_TIM_HANDLE);
}

void bsp_ws2812_transmit(void)
{

	// 先把之前可能残留的 DMA 传输停掉，顺便把状态从 BUSY 拉回 READY
	HAL_TIM_PWM_Stop_DMA(&BSP_WS2812_TIM_HANDLE, BSP_WS2812_TIM_CHANNEL);

	HAL_StatusTypeDef st = HAL_TIM_PWM_Start_DMA(&BSP_WS2812_TIM_HANDLE,BSP_WS2812_TIM_CHANNEL,(uint32_t *)ws2812_data,BSP_WS2812_DATA_NUM);
	if (st != HAL_OK) {
		HAL_Delay(500);
		// 这里打个断点，看 st 是不是 HAL_ERROR 或 HAL_BUSY
	}
	// HAL_TIM_PWM_Start_DMA(&BSP_WS2812_TIM_HANDLE,BSP_WS2812_TIM_CHANNEL,(uint32_t *)ws2812_data,BSP_WS2812_DATA_NUM);
}


void bsp_ws2812_off(void)
{
	uint16_t i;
	for (i = 0; i < BSP_WS2812_PIXEL_NUM * 24; i++)
	{
		ws2812_data[i] = WS0; // 写入逻辑0的占空比
	}
	for (i = BSP_WS2812_PIXEL_NUM * 24; i < BSP_WS2812_DATA_NUM; i++)
	{
		ws2812_data[i] = 0; // 占空比比为0，全为低电平
	}
}

void bsp_ws2812_set_all(uint32_t color)
{
	uint8_t n_R = (color & 0x00FF0000) >> 16;
	uint8_t n_G = (color & 0x0000FF00) >> 8;
	uint8_t n_B = (color & 0x000000FF) >> 0;
	uint16_t i, j;
	uint8_t dat[24];
	// 将RGB数据进行转换
	for (i = 0; i < 8; i++)
	{
		dat[i] = ((n_G & 0x80) ? WS1 : WS0);
		n_G <<= 1;
	}
	for (i = 0; i < 8; i++)
	{
		dat[i + 8] = ((n_R & 0x80) ? WS1 : WS0);
		n_R <<= 1;
	}
	for (i = 0; i < 8; i++)
	{
		dat[i + 16] = ((n_B & 0x80) ? WS1 : WS0);
		n_B <<= 1;
	}
	for (i = 0; i < BSP_WS2812_PIXEL_NUM; i++)
	{
		for (j = 0; j < 24; j++)
		{
			ws2812_data[i * 24 + j] = dat[j];
		}
	}
	for (i = BSP_WS2812_PIXEL_NUM * 24; i < BSP_WS2812_DATA_NUM; i++)
		ws2812_data[i] = 0; // 占空比比为0，全为低电平
}

void bsp_ws2812_set_pixel(uint16_t n, uint32_t color)
{
	uint8_t i;

	if (n < BSP_WS2812_PIXEL_NUM)
	{
		for (i = 0; i < 24; ++i)
			ws2812_data[24 * n + i] = ((color << i) & 0X800000) ? WS1 : WS0;
	}
}

static inline uint32_t ws2812_combine(uint8_t red, uint8_t green, uint8_t blue) { return green << 16 | red << 8 | blue; }

static inline uint32_t ws2812_wheel(uint8_t wheel_pos)
{
	wheel_pos = 255 - wheel_pos;
	if (wheel_pos < 85)
	{
		return ws2812_combine(255 - wheel_pos * 3, 0, wheel_pos * 3);
	}
	if (wheel_pos < 170)
	{
		wheel_pos -= 85;
		return ws2812_combine(0, wheel_pos * 3, 255 - wheel_pos * 3);
	}
	wheel_pos -= 170;
	return ws2812_combine(wheel_pos * 3, 255 - wheel_pos * 3, 0);
}

void bsp_ws2812_rainbow(uint8_t period)
{
	uint32_t timestamp = HAL_GetTick();
	uint16_t i;
	static uint8_t j;
	static uint32_t next_time = 0;

	uint32_t flag = 0;
	if (next_time < period)
	{
		if ((uint64_t)timestamp + period - next_time > 0)
		{
			flag = 1;
		}
	}
	else if (timestamp > next_time)
	{
		flag = 1;
	}
	if (flag) // && (timestamp - next_time < wait*5))
	{
		j++;
		next_time = timestamp + period;
		for (i = 0; i < BSP_WS2812_PIXEL_NUM; i++)
		{
			bsp_ws2812_set_pixel(i, ws2812_wheel((i + j) & 255));
		}
	}
}

void bsp_ws2812_rainbow_cycle(uint8_t loop_period)
{
	uint32_t timestamp = HAL_GetTick();
	uint16_t i;
	static uint8_t j;
	static uint32_t next_time = 0;

	static uint8_t loop = 0;
	if (loop == 0)
	{
		next_time = timestamp;
	}
	loop = 1; // 首次调用初始化

	if ((timestamp > next_time)) // && (timestamp - next_time < wait*5))
	{
		j++;
		next_time = timestamp + loop_period;
		for (i = 0; i < BSP_WS2812_PIXEL_NUM; i++)
		{
			bsp_ws2812_set_pixel(i, ws2812_wheel(((i * 256 / (BSP_WS2812_PIXEL_NUM)) + j) & 255));
		}
	}
}

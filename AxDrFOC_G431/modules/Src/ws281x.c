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
	//确保定时器TIM2初始化开启
	if (HAL_TIM_Base_GetState(&BSP_WS2812_TIM_HANDLE) != HAL_TIM_STATE_READY)
	{
		BSP_WS2812_TIM_INIT();
	}

	//启动定时器的计数功能（base）
	HAL_TIM_Base_Start(&BSP_WS2812_TIM_HANDLE);
}

void bsp_ws2812_transmit(void)
{

	// 先把之前可能残留的 DMA 传输停掉，顺便把状态从 BUSY 拉回 READY
	HAL_TIM_PWM_Stop_DMA(&BSP_WS2812_TIM_HANDLE, BSP_WS2812_TIM_CHANNEL);

	//数据发送 + 调试
	HAL_StatusTypeDef st = HAL_TIM_PWM_Start_DMA(&BSP_WS2812_TIM_HANDLE,BSP_WS2812_TIM_CHANNEL,(uint32_t *)ws2812_data,BSP_WS2812_DATA_NUM);
	if (st != HAL_OK) {
		HAL_Delay(500);
		// 这里打个断点，看 st 是不是 HAL_ERROR 或 HAL_BUSY
	}

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

static inline uint32_t ws2812_combine(uint8_t red, uint8_t green, uint8_t blue)
{
	return green << 16 | red << 8 | blue;
}

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



///USER FUNCTION

/**
 * @brief  设置整条灯带的颜色和亮度
 * @param  r_ratio    基础 R 分量 (0~255)，表示比例，不是最终亮度
 * @param  g_ratio    基础 G 分量 (0~255)
 * @param  b_ratio    基础 B 分量 (0~255)
 * @param  level     亮度等级，1~10，对应 10%~100%
 */
void RGB_SetColor(uint8_t r_ratio,
				  uint8_t g_ratio,
				  uint8_t b_ratio,
				  uint8_t level)
{
	if (level < 0) level = 0;
	if (level > 10) level = 10;

	// 1. 比例全为 0，直接关灯
	uint8_t max_ratio = r_ratio;
	if (g_ratio > max_ratio) max_ratio = g_ratio;
	if (b_ratio > max_ratio) max_ratio = b_ratio;

	if (max_ratio == 0)//比例全零关闭
	{
		bsp_ws2812_off();
		bsp_ws2812_transmit();
		return;
	}

	// 2. 当前亮度等级下的“最大通道值”
	//    level:1~10 → 映射到 25~255，你可以自己调这个常数
	uint16_t max_val = (uint16_t)(255u * level / 10u); // 10 级就是 255

	// 3. 按比例把三个通道缩放到 0~max_val 之间
	uint8_t r = (uint8_t)((uint32_t)r_ratio * max_val / max_ratio);
	uint8_t g = (uint8_t)((uint32_t)g_ratio * max_val / max_ratio);
	uint8_t b = (uint8_t)((uint32_t)b_ratio * max_val / max_ratio);

	// 4. 拼成 0x00RRGGBB 调底层
	uint32_t color = ((uint32_t)r << 16) |
					 ((uint32_t)g << 8)  |
					 ((uint32_t)b);

	bsp_ws2812_set_all(color);
	bsp_ws2812_transmit();
}


/**
 * @brief  彩虹跑马灯效果（整条灯带一圈彩虹转动），带亮度控制
 * @param  level        亮度等级 1~10（1 最暗，10 最亮）
 * @param  speed_ms     彩虹前进一次的时间间隔（ms），值越小越快
 *
 * @note   这个函数是“任务函数”，需要在 while(1) 里被反复调用，
 *         它内部自己根据 HAL_GetTick() 控制刷新节奏。
 */
void RGB_RainbowCycle(uint8_t level, uint16_t speed_ms)
{
	static uint8_t  j = 0;           // 彩虹偏移
	static uint8_t  inited = 0;
	static uint32_t next_time = 0;

	uint32_t now = HAL_GetTick();

	if (!inited)
	{
		inited = 1;
		next_time = now;
	}

	if (level < 1) level = 1;
	if (level > 10) level = 10;

	// 没到时间就直接返回（非阻塞）
	if (now < next_time)
		return;

	next_time = now + speed_ms;
	j++;

	// 当前亮度等级下的“最大通道值”，10 级 ≈ 255，全亮
	uint16_t max_val = (uint16_t)(255u * level / 10u);

	for (uint16_t i = 0; i < BSP_WS2812_PIXEL_NUM; ++i)
	{
		// 这一颗灯在彩虹上的位置（空间上平铺一圈 0~255）
		uint8_t pos = (uint8_t)(((uint32_t)i * 256u / BSP_WS2812_PIXEL_NUM + j) & 0xFF);

		// 轮盘上拿一个颜色（GRB 24bit）
		uint32_t c = ws2812_wheel(pos);

		// 拆成“比例”
		uint8_t g_ratio = (c >> 16) & 0xFF;
		uint8_t r_ratio = (c >> 8)  & 0xFF;
		uint8_t b_ratio = (c >> 0)  & 0xFF;

		// 按亮度等级缩放到 0 ~ max_val
		uint8_t r = (uint8_t)((uint32_t)r_ratio * max_val / 255u);
		uint8_t g = (uint8_t)((uint32_t)g_ratio * max_val / 255u);
		uint8_t b = (uint8_t)((uint32_t)b_ratio * max_val / 255u);

		// 重新组合成 GRB 给 set_pixel
		bsp_ws2812_set_pixel(i, ws2812_combine(r, g, b));
	}

	// 刷出去
	bsp_ws2812_transmit();
}



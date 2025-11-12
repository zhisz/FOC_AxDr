#include "graphics.h"
#include "fonts.h"

#include "stdbool.h"

#ifdef USE_MONITOR

static bool IS_LCD_IN_TX_STATE = false;

void bsp_lcd_tx_state_idle(void) { IS_LCD_IN_TX_STATE = false; }

// static uint16_t framebuffer[TotalPixel + 512];
static uint16_t framebuffer_line[LCD_W];

// uint16_t framebuffer[240 * 320 * 1 + 512];

static void LCD_WriteCommand(uint8_t cmd)
{
    LCD_DC_Command;
    HAL_SPI_Transmit(&BSP_LCD_SPI_HANDLE, &cmd, 1, 1000);
    // HAL_SPI_Transmit_DMA(&BSP_LCD_SPI_HANDLE, &cmd, 1);
}

static void LCD_WriteByte(uint8_t data)
{
    LCD_DC_Data;
    HAL_SPI_Transmit(&BSP_LCD_SPI_HANDLE, &data, 1, 1000);
    // HAL_SPI_Transmit_DMA(&BSP_LCD_SPI_HANDLE, &data, 1);
}

static void LCD_WriteHalfWord(uint16_t data)
{
    uint8_t data_byte[2];
    data_byte[0] = data >> 8;
    data_byte[1] = data;
    LCD_DC_Data;
    HAL_SPI_Transmit(&BSP_LCD_SPI_HANDLE, data_byte, 2, 1000);
    // HAL_SPI_Transmit_DMA(&BSP_LCD_SPI_HANDLE, data_byte, 2);
}

void bsp_lcd_set_region(uint16_t x, uint16_t x2, uint16_t y, uint16_t y2)
{
    LCD_WriteCommand(0x2A);
    LCD_WriteHalfWord(x + LCD_X_BIAS);
    LCD_WriteHalfWord(x2 + LCD_X_BIAS);
    LCD_WriteCommand(0x2B);
    LCD_WriteHalfWord(y + LCD_Y_BIAS);
    LCD_WriteHalfWord(y2 + LCD_Y_BIAS);
    LCD_WriteCommand(0x2C);
}

void bsp_lcd_fill(uint16_t color)
{
    uint8_t data[2];
    data[0] = color >> 8;
    data[1] = color;

    bsp_lcd_set_region(0, LCD_W-1, 0, LCD_H-1);
    LCD_DC_Data;

    for(uint32_t i=0; i<LCD_W*LCD_H; i++)
        HAL_SPI_Transmit(&BSP_LCD_SPI_HANDLE, data, 2, 10);
}



// void bsp_lcd_transmit_buffer(void)
// {
//     uint32_t i = 0;
//     bsp_lcd_set_region(0, LCD_W - 1, 0, LCD_H - 1);
//     LCD_DC_Data;
//     // HAL_SPI_Transmit(&hspi3, (uint8_t *)framebuffer, SizeOfGram, 1000);
//
//     while (IS_LCD_IN_TX_STATE)
//         ;
//
//     HAL_SPI_Transmit_DMA(&hspi3, (uint8_t *)framebuffer, SizeOfGram);
//
//     // extern DMA_HandleTypeDef hdma_spi3_tx;
//     // while (HAL_DMA_GetState(&hdma_spi3_tx) == HAL_DMA_STATE_BUSY)
//     //     ;
// }

void bsp_lcd_fill_region(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t color)
{
    for(uint32_t y = y1; y <= y2; y++)
    {
        for(uint32_t x = x1; x <= x2; x++)
            framebuffer_line[x - x1] = color;

        bsp_lcd_set_region(x1, x2, y, y);
        LCD_DC_Data;
        HAL_SPI_Transmit(&BSP_LCD_SPI_HANDLE, (uint8_t*)framebuffer_line, (x2 - x1 + 1) * 2, 1000);
    }
}

// void bsp_lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
// {
//     LCD_SetRegion(x, x + 1, y, y + 1);
//     LCD_WriteHalfWord(color);
// }

void bsp_lcd_draw_point(uint32_t x, uint32_t y, uint16_t color)
{
    bsp_lcd_set_region(x, x, y, y);
    LCD_WriteHalfWord(color);
}

// glib库中的画线函数，可以画斜线，线两端分别是(x1, y1)和(x2, y2)
void bsp_lcd_draw_line(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, uint16_t color)
{
    int dx, dy, e;
    dx = x2 - x1;
    dy = y2 - y1;

    if (dx >= 0)
    {
        if (dy >= 0) // dy>=0
        {
            if (dx >= dy) // 1/8 octant
            {
                e = dy - dx / 2;
                while (x1 <= x2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        y1 += 1;
                        e -= dx;
                    }
                    x1 += 1;
                    e += dy;
                }
            }
            else // 2/8 octant
            {
                e = dx - dy / 2;
                while (y1 <= y2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        x1 += 1;
                        e -= dy;
                    }
                    y1 += 1;
                    e += dx;
                }
            }
        }
        else // dy<0
        {
            dy = -dy;     // dy=abs(dy)
            if (dx >= dy) // 8/8 octant
            {
                e = dy - dx / 2;
                while (x1 <= x2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        y1 -= 1;
                        e -= dx;
                    }
                    x1 += 1;
                    e += dy;
                }
            }
            else // 7/8 octant
            {
                e = dx - dy / 2;
                while (y1 >= y2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        x1 += 1;
                        e -= dy;
                    }
                    y1 -= 1;
                    e += dx;
                }
            }
        }
    }
    else // dx<0
    {
        dx = -dx;    // dx=abs(dx)
        if (dy >= 0) // dy>=0
        {
            if (dx >= dy) // 4/8 octant
            {
                e = dy - dx / 2;
                while (x1 >= x2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        y1 += 1;
                        e -= dx;
                    }
                    x1 -= 1;
                    e += dy;
                }
            }
            else // 3/8 octant
            {
                e = dx - dy / 2;
                while (y1 <= y2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        x1 -= 1;
                        e -= dy;
                    }
                    y1 += 1;
                    e += dx;
                }
            }
        }
        else // dy<0
        {
            dy = -dy;     // dy=abs(dy)
            if (dx >= dy) // 5/8 octant
            {
                e = dy - dx / 2;
                while (x1 >= x2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        y1 -= 1;
                        e -= dx;
                    }
                    x1 -= 1;
                    e += dy;
                }
            }
            else // 6/8 octant
            {
                e = dx - dy / 2;
                while (y1 >= y2)
                {
                    bsp_lcd_draw_point(x1, y1, color);
                    if (e > 0)
                    {
                        x1 -= 1;
                        e -= dy;
                    }
                    y1 -= 1;
                    e += dx;
                }
            }
        }
    }
}

void bsp_lcd_draw_from_img(uint32_t x, uint32_t y, const uint8_t *IMG, uint8_t IMG_W, uint8_t IMG_H, uint16_t color)
{
    uint32_t TotalCounter = 0;
    uint32_t BitCounter = 0;
    uint32_t Buffer = 0;
    uint32_t tempx = x;
    uint32_t tempy = y;

    IMG_W += IMG_W % 8;

    IMG_H += IMG_H % 8;

    for (TotalCounter = 0; TotalCounter < IMG_W * IMG_H / 8;)
    {
        Buffer = *(IMG + TotalCounter);
        for (BitCounter = 0; BitCounter < 8;)
        {
            if (Buffer & 0x01)
            {
                bsp_lcd_draw_point(tempx, tempy, color);
            }

            tempx++;
            if (tempx >= x + IMG_W)
            {
                tempx = x;
                tempy++;
            }
            Buffer >>= 1;
            BitCounter++;
        }

        TotalCounter++;
    }
}

void bsp_lcd_draw_from_img_bgc(uint32_t x, uint32_t y, const uint8_t *IMG, uint8_t IMG_W, uint8_t IMG_H, uint16_t color, uint16_t BGcolor)
{

    uint32_t TotalCounter = 0;
    uint32_t BitCounter = 0;
    uint32_t Buffer = 0;
    uint32_t tempx = x;
    uint32_t tempy = y;

    IMG_W += IMG_W % 8;

    IMG_H += IMG_H % 8;

    for (TotalCounter = 0; TotalCounter < IMG_W * IMG_H / 8;)
    {
        Buffer = *(IMG + TotalCounter);
        for (BitCounter = 0; BitCounter < 8;)
        {
            if (Buffer & 0x01)
            {
                bsp_lcd_draw_point(tempx, tempy, color);
            }
            else
            {
                bsp_lcd_draw_point(tempx, tempy, BGcolor);
            }

            tempx++;
            if (tempx >= x + IMG_W)
            {
                tempx = x;
                tempy++;
            }
            Buffer >>= 1;
            BitCounter++;
        }

        TotalCounter++;
    }
}

void bsp_lcd_printf(uint32_t x, uint32_t y, uint32_t font_num, uint16_t color, char *fmt, ...)
{
    uint32_t temp = 0;

    uint32_t tempx = x;
    uint32_t tempy = y;
    uint32_t font_width;
    uint32_t font_height;
    uint32_t font_unit_size;
    uint8_t buf[100];

    unsigned char *pfont = NULL;
    uint16_t len = strlen((const char *)fmt);
    if (len > 100)
    {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)buf, fmt, ap);
    va_end(ap);
    switch (font_num)
    {
    default:
    case 1:
        /* code */
        pfont = (unsigned char *)ASCII_8_16;
        font_width = 8;
        font_height = 16;
        font_unit_size = 16;
        break;
    case 2:
        /* code */
        pfont = (unsigned char *)ASCII_12_32;
        font_width = 12;
        font_height = 32;
        font_unit_size = 96;
        break;
    case 3:
        pfont = (unsigned char *)JetBrainMono;
        font_width = 8;
        font_height = 16;
        font_unit_size = 16;
        break;
    }
    for (temp = 0; temp < len;)
    {
        if (tempx >= LCD_W - font_width)
        {
            tempx = x;
            tempy += font_height;
        }
        if (buf[temp] == '\r' | buf[temp] == '\n')
        {
            temp++;
            tempx = x;
            tempy += font_height;
        }
        if (buf[temp] == '\0')
        {
            break;
        }
        bsp_lcd_draw_from_img(tempx, tempy, pfont + ((buf[temp] - 32) * (font_unit_size)), font_width, font_height, color);
        tempx += font_width;

        temp++;
    }
}

void bsp_lcd_printf_bgc(uint32_t x, uint32_t y, uint32_t font_num, uint16_t color, uint16_t BGcolor, char *fmt, ...)
{
    uint32_t temp = 0;

    uint32_t tempx = x;
    uint32_t tempy = y;
    uint32_t font_width;
    uint32_t font_height;
    uint32_t font_unit_size;

    //    uint32_t xnum;
    //    uint32_t ynum;
    uint8_t buf[200];

    unsigned char *pfont = NULL;
    uint16_t len = strlen((const char *)fmt);
    if (len > 200)
    {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)buf, fmt, ap);
    va_end(ap);
    switch (font_num)
    {
    default:
    case 1:
        /* code */
        pfont = (unsigned char *)ASCII_8_16;
        font_width = 8;
        font_height = 16;
        font_unit_size = 16;
        break;
    case 2:
        /* code */
        pfont = (unsigned char *)ASCII_12_32;
        font_width = 12;
        font_height = 32;
        font_unit_size = 96;
        break;
    case 3:
        pfont = (unsigned char *)JetBrainMono;
        font_width = 8;
        font_height = 16;
        font_unit_size = 16;
        break;
    }

    //    xnum = (LCD_W - x) / font_width;
    //    ynum = 1 + len / xnum;
    for (temp = 0; temp < len;)
    {
        if (tempx >= LCD_W - font_width)
        {
            tempx = x;
            tempy += font_height;
        }
        if (buf[temp] == '\r' | buf[temp] == '\n')
        {
            temp++;
            tempx = x;
            tempy += font_height;
        }
        if (buf[temp] == '\0')
        {
            break;
        }
        bsp_lcd_draw_from_img_bgc(tempx, tempy, pfont + ((buf[temp] - 32) * (font_unit_size)), font_width, font_height, color, BGcolor);
        tempx += font_width;

        temp++;
    }
}

void bsp_lcd_draw_pic(const uint16_t *pic, uint32_t W, uint32_t H)
{
    uint32_t y;
    uint32_t line_size = W * 2;    // 每行字节数 = 像素个数 * 2

    for (y = 0; y < H; y++)
    {
        bsp_lcd_set_region(0, W - 1, y, y);   // 设置写入区域为第 y 行
        LCD_DC_Data;
        HAL_SPI_Transmit(&BSP_LCD_SPI_HANDLE,
                         (uint8_t *)(pic + y * W),
                         line_size,
                         1000);
    }
}


static void LCD_CirclePlot(uint32_t x, uint32_t y, uint32_t xi, uint32_t yi, uint16_t color)
{
    bsp_lcd_draw_point(x + xi, y + yi, color);
    bsp_lcd_draw_point(x + yi, y + xi, color);
    bsp_lcd_draw_point(x - xi, y + yi, color);
    bsp_lcd_draw_point(x - yi, y + xi, color);
    bsp_lcd_draw_point(x - xi, y - yi, color);
    bsp_lcd_draw_point(x - yi, y - xi, color);
    bsp_lcd_draw_point(x + xi, y - yi, color);
    bsp_lcd_draw_point(x + yi, y - xi, color);
}

void bsp_lcd_fast_circle(uint32_t x, uint32_t y, uint32_t r, uint32_t color)
{
    uint16_t Tcolor = TransColor888to565(color);

    int xi;
    int yi;
    int di;
    if ((int32_t)(x - r) < 0 || x + r > LCD_W || (int32_t)(y - r) < 0 || y + r > LCD_H)
    {
        return;
    }
    di = 0 - (r >> 1);
    xi = 0;
    yi = r;
    while (yi >= xi)
    {
        LCD_CirclePlot(x, y, xi, yi, Tcolor);
        xi++;
        if (di < 0)
        {
            di += xi;
        }
        else
        {
            yi--;
            di += xi - yi;
        }
    }
}

void bsp_lcd_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint32_t color)
{
    uint16_t Tcolor = TransColor888to565(color);
    int x, y;
    int deltax, deltay;
    int d;
    int xi;
    x = 0;
    y = r;
    deltax = 3;
    deltay = 2 - r - r;
    d = 1 - r;

    bsp_lcd_draw_point(x + x0, y + y0, Tcolor);
    bsp_lcd_draw_point(x + x0, -y + y0, Tcolor);
    for (xi = -r + x0; xi <= r + x0; xi++)
        bsp_lcd_draw_point(xi, y0, Tcolor); // 水平线填充
    while (x < y)
    {
        if (d < 0)
        {
            d += deltax;
            deltax += 2;
            x++;
        }
        else
        {
            d += (deltax + deltay);
            deltax += 2;
            deltay += 2;
            x++;
            y--;
        }
        for (xi = -x + x0; xi <= x + x0; xi++)
        {
            bsp_lcd_draw_point(xi, -y + y0, Tcolor);
            bsp_lcd_draw_point(xi, y + y0, Tcolor); // 扫描线填充
        }
        for (xi = -y + x0; xi <= y + x0; xi++)
        {
            bsp_lcd_draw_point(xi, -x + y0, Tcolor);
            bsp_lcd_draw_point(xi, x + y0, Tcolor); // 扫描线填充其量
        }
    }
}

static int32_t ocs_data[LCD_W];
static uint32_t schedule;
static int32_t ocs_max, ocs_min;
static fp32 ocs_propotion = 1.0f;
void bsp_lcd_ocs(int32_t data)
{
    if (data < LCD_H / 2 || data > -LCD_H / 2)
    {
        ocs_data[schedule] = data;
    }
    else if (data > LCD_H / 2)
    {
        ocs_data[schedule] = LCD_H / 2;
    }
    else if (data < -LCD_H / 2)
    {
        ocs_data[schedule] = -LCD_H / 2;
    }
    for (uint32_t i = 0; i < LCD_W;)
    {
        //        // bsp_lcd_draw_point(i, ocs_data[i] + 80, 0x0000);
        //        if (data >= LCD_H / 2)
        //        {
        //            ocs_max = data;
        //            ocs_propotion = LCD_H / (ocs_max - ocs_min);
        //        }
        //        else if (data <= -LCD_H / 2)
        //        {
        //            ocs_min = data;
        //            ocs_propotion = LCD_H / (ocs_max - ocs_min);
        //        }

        if (i == 0)
        {
            bsp_lcd_draw_point(i, ocs_propotion * ocs_data[i] + LCD_H / 2, 0x0000);
            i++;
            continue;
        }
        if (i < schedule)
        {
            bsp_lcd_draw_line(i - 1, i, (uint32_t)(ocs_propotion * ocs_data[i - 1] + LCD_H / 2), (uint32_t)(ocs_propotion * ocs_data[i] + LCD_H / 2), 0x0000);
        }
        i++;
    }

    schedule++;
    if (schedule >= LCD_W)
    {
        schedule = 0;
        // BSP_LTDC_Clear(BSP_LTDC_COLOR_RED);
        // memset(ocs_data, 0, LCD_W);
    }
}

void bsp_lcd_init(void)
{
    LCD_RES_Clr();
    HAL_Delay(100);
    LCD_RES_Set();
    HAL_Delay(100);

    LCD_CS_Clr();

    LCD_WriteCommand(0x36);
    LCD_WriteByte(0x70);  // 显示方向：根据需要调为0x00/0x70/0xC0/0xA0

    LCD_WriteCommand(0x3A);
    LCD_WriteByte(0x05);  // 16-bit color

    LCD_WriteCommand(0xB2);
    LCD_WriteByte(0x0C);
    LCD_WriteByte(0x0C);
    LCD_WriteByte(0x00);
    LCD_WriteByte(0x33);
    LCD_WriteByte(0x33);

    LCD_WriteCommand(0xB7);
    LCD_WriteByte(0x35);

    LCD_WriteCommand(0xBB);
    LCD_WriteByte(0x19);

    LCD_WriteCommand(0xC0);
    LCD_WriteByte(0x2C);

    LCD_WriteCommand(0xC2);
    LCD_WriteByte(0x01);

    LCD_WriteCommand(0xC3);
    LCD_WriteByte(0x12);

    LCD_WriteCommand(0xC4);
    LCD_WriteByte(0x20);

    LCD_WriteCommand(0xC6);
    LCD_WriteByte(0x0F);

    LCD_WriteCommand(0xD0);
    LCD_WriteByte(0xA4);
    LCD_WriteByte(0xA1);

    LCD_WriteCommand(0xE0);
    uint8_t seq1[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54,
                      0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
    HAL_SPI_Transmit(&hspi3, seq1, sizeof(seq1), 100);

    LCD_WriteCommand(0xE1);
    uint8_t seq2[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44,
                      0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
    HAL_SPI_Transmit(&hspi3, seq2, sizeof(seq2), 100);

    LCD_WriteCommand(0x21);  // 显示反相
    LCD_WriteCommand(0x11);  // 退出睡眠
    HAL_Delay(120);
    LCD_WriteCommand(0x29);  // 开显示

    LCD_CS_Set();
}


#endif

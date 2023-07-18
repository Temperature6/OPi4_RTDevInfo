//
// Created by AQin on 2023/2/8.
//

#include "SSD1306_IIC.h"
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "ConMemRW.h"
#include "UserCfg.h"

#define PAGE_START_ADDR         0xB0
#define LOWER_SATRT_COL_ADDR    0x00
#define UPPER_SATRT_COL_ADDR    0x10

#define BUFFER_SIZE             (128 * 8)

#define MAX(n1, n2)             ((n1) > (n2) ? (n1) : (n2))
#define MIN(n1, n2)             ((n1) < (n2) ? (n1) : (n2))

/*Display Command*/
#define DISPLAY_OFF             0xAE
#define DISPLAY_ON              0xAF

int fd = 0;

uint8_t SSD1306_Buffer[BUFFER_SIZE];

void WriteByte(uint8_t Addr, uint8_t Data);
void WriteMem(uint8_t Addr, uint8_t* pData, uint16_t Len);
void WriteCmd(uint8_t cmd);

void SSD1306_Init()
{
    /*等待启动*/
    //HAL_Delay(100);
    wiringPiSetupSys();
    fd = wiringPiI2CSetupInterface(LINUX_IIC_FILE, OLED_ADDR);
    /*初始化*/
    WriteCmd(0xAE);     //关闭显示

    WriteCmd(0xA8);     //设置多路复用比例，默认A8h，63
    WriteCmd(0x3F);

    WriteCmd(0xD3);     //设置显示偏移，默认D3h，0
    WriteCmd(0x00);

    WriteCmd(0x40);     //设置显示起始行
    //水平翻转
#ifdef MIRROR_HORIZONTAL
    WriteCmd(0xA0);
#else
    WriteCmd(0xA1);
#endif //MIRROR_HORIZONTAL
    //垂直翻转
#ifdef MIRROR_VERTICAL
    WriteCmd(0xC0);
#else
    WriteCmd(0xC8);
#endif //MIRROR_VERTICAL
    WriteCmd(0xDA);     //设置COM引脚硬件配置，默认DAh，02
    WriteCmd(0x12);

    WriteCmd(0x81);     //设置对比度控制，默认81h，7Fh
    WriteCmd(0x7F);

    WriteCmd(0xA4);     //设置显示内容来自于GDDRAM

    WriteCmd(0xA6);     //设置不反转颜色

    WriteCmd(0xD5);     //设置晶振频率，默认D5h，80h
    WriteCmd(0x80);

    WriteCmd(0x8D);     //设置电荷泵稳压器，默认8Dh，14h
    WriteCmd(0x14);

    WriteCmd(0xAF);     //打开屏幕


    SSD1306_ClearScreen();
    SSD1306_UpdateScreen();
}

void WriteByte(uint8_t Addr, uint8_t Data)
{
    wiringPiI2CWriteReg8(fd, Addr, Data);
}

void WriteMem(uint8_t Addr, uint8_t* pData, uint16_t Len)
{
    wiringPiI2CWirteRegs(fd, Addr, pData, Len);
}

void WriteCmd(uint8_t cmd)
{
    WriteByte(0x00, cmd);
}

void SSD1306_InvertColor(SSD1306_State state)
{
    if (state == SSD1306_ENABLE)
        WriteCmd(0xA7);     //反色显示
    else
        WriteCmd(0xA6);     //正常显示
}

void SSD1306_ClearScreen()
{
    memset(SSD1306_Buffer, 0x00, BUFFER_SIZE);

#ifndef FULL_REFRESH
    SSD1306_UpdateScreen();
#endif //FULL_REFRESH
}

void SSD1306_FullScreen()
{
    memset(SSD1306_Buffer, 0xFF, BUFFER_SIZE);

#ifndef FULL_REFRESH
    SSD1306_UpdateScreen();
#endif //FULL_REFRESH
}

void SSD1306_UpdateScreen()
{
    for (uint8_t j = 0; j < 4; j++)
    {
        for(uint8_t i = 0; i < 64/8; i++)
        {
            WriteCmd(PAGE_START_ADDR + i);
            WriteCmd(LOWER_SATRT_COL_ADDR + ((32 * j) & 0x0F));
            WriteCmd(UPPER_SATRT_COL_ADDR + (((32 * j) & 0xF0) >> 4));
            WriteMem(0x40, &SSD1306_Buffer[128*i + j * 32], 32);
        }
    }
}

void SSD1306_TurnOn(SSD1306_State state)
{
    if (state == SSD1306_ENABLE)
        WriteCmd(0xAF);
    else
        WriteCmd(0xAE);
}

void SSD1306_PutChar(uint8_t x, uint8_t y, char c, MatrixFont font, SSD1306_Color color)
{
    uint32_t byte;

    //不支持的字符
    if (c < 32 || c > 126)
        return;

    //判断超出屏幕边界
    if (128 < (x + font.width) || 64 < (y + font.height))
        return;

    for(uint32_t i = 0; i < font.height; i++)
    {
        byte = font.bitmap[(c - 32) * font.height + i];     //取一个数据
        for(uint32_t j = 0; j < font.width; j++)
        {
            SSD1306_DrawPixel(x + j, y + i, ((byte << j) & 0x8000) == 0 ? !color : color);
        }
    }
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_Color color)
{
    if (x >= 128 || y >= 64)    //超出范围
        return;

    uint8_t COM_y = y / 8;
    uint8_t Bit_y = y % 8;

    if (color == White) //白色
        SSD1306_Buffer[128 * COM_y + x] |= (0x01 << Bit_y);
    else                //黑色
        SSD1306_Buffer[128 * COM_y + x] &= ~(0x01 << Bit_y);

#ifndef FULL_REFRESH
    WriteCmd(0xB0 + COM_y);
    WriteCmd(0x00 + x % 16);
    WriteCmd(0x10 + x / 16);
    WriteByte(0x40, SSD1306_Buffer[128 * COM_y + x]);
#endif //FULL_REFRESH

}

void SSD1306_DrawBitMap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height, SSD1306_Color color)
{
    uint8_t byte_width = (width + 7) / 8;   //计算图像宽度（bytes）
    uint8_t byte = 0;

    for (int16_t j = 0; j < height; j++, y++)
    {
        for (int16_t i = 0; i < width; i++)
        {
            if (i & 7)
                byte <<= 1;
            else
                byte = bitmap[j * byte_width + i / 8];
            if (byte & 0x80)
                SSD1306_DrawPixel(x + i, y, color);
        }
    }
}

void SSD1306_PutString(uint8_t x, uint8_t y, char *c, MatrixFont font, SSD1306_Color color)
{
    uint16_t len = strlen(c);
    for (uint16_t i = 0; i < len; i++)
    {
        SSD1306_PutChar(x + i * font.width, y, c[i], font, color);
    }

}

void SSD1306_SetContrast(uint8_t val)
{
    WriteCmd(0x81);
    WriteCmd(val);
}

void SSD1306_DrawLine(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, SSD1306_Color color)
{

    if (start_x == end_x)   //斜率不存在，特殊处理
    {
        uint8_t sy = start_y > end_y ? end_y : start_y;
        uint8_t ey = start_y < end_y ? end_y : start_y;

        for (uint8_t i = sy; i <= ey; i++)
        {
            SSD1306_DrawPixel(start_x, i, color);
        }
    }
    else
    {
        double k = 1.0 * (start_y - end_y) / (start_x - end_x);
        double b = start_y - k * start_x;

        uint8_t sx = start_x > end_x ? end_x : start_x;
        uint8_t ex = start_x < end_x ? end_x : start_x;

        for (uint8_t i = sx; i <= ex; i++)
        {
            SSD1306_DrawPixel(i, (uint8_t)(k * i + b), color);
        }
    }
}

void SSD1306_DrawRectangle1(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, SSD1306_Color color)
{
    SSD1306_DrawLine(start_x, start_y, end_x, start_y, color);
    SSD1306_DrawLine(start_x, start_y, start_x, end_y, color);
    SSD1306_DrawLine(start_x, end_y, end_x, end_y, color);
    SSD1306_DrawLine(end_x, start_y, end_x, end_y, color);
}

void SSD1306_DrawRectangle2(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height, SSD1306_Color color)
{
    SSD1306_DrawRectangle1(start_x, start_y, start_x + width, start_y + height, color);
}

void SSD1306_FillRect1(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, SSD1306_Color color)
{
    uint8_t sx = MIN(start_x, end_x);
    uint8_t sy = MIN(start_y, end_y);
    uint8_t ex = MAX(start_x, end_x);
    uint8_t ey = MAX(start_y, end_y);

    for (uint8_t i = sx; i <= ex; i++)
        for (uint8_t j = sy; j <= ey; j++)
            SSD1306_DrawPixel(i, j, color);
}

void SSD1306_FillRect2(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height, SSD1306_Color color)
{
    SSD1306_FillRect1(start_x, start_y, start_x + width, start_y + height, color);
}

// void SSD1306_DrawCircle(uint8_t center_x, uint8_t center_y, uint8_t rad, SSD1306_Color color)
// {
// //#warning "This Function don't Work Well"
//     int16_t range_sx = center_x - rad;
//     int16_t range_ex = center_x + rad;
//     double temp = 0.0;
//     for (int16_t i = range_sx; i <= range_ex; i++)
//     {

//         //temp = sqrt(abs((rad + i - center_x) * (rad - i + center_x)));
//         SSD1306_DrawPixel(i, (uint8_t)(center_y + temp), color);
//         SSD1306_DrawPixel(i, (uint8_t)(center_y - temp), color);
//     }
// }

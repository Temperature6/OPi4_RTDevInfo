//
// Created by AQin on 2023/2/8.
//

#ifndef SSD1306_IIC_H
#define SSD1306_IIC_H


#include "memory.h"
#include "math.h"
#include "stdlib.h"
#include "MatrixFont.h"
#include "stdint.h"

#define OLED_USE_IIC                hi2c1
#define OLED_ADDR                   0x3C
//#define LINUX_IIC_FILE              "/dev/i2c-8"
#define FULL_REFRESH
//#define MIRROR_VERTICAL
//#define MIRROR_HORIZONTAL


typedef enum
{
    White = 1,
    Black = !White
}SSD1306_Color;

typedef enum
{
    SSD1306_ENABLE = 0,
    SSD1306_DISABLE = !SSD1306_ENABLE
}SSD1306_State;

/*Public Function*/
void SSD1306_Init();

//Graphic Function
//void SSD1306_DrawCircle(uint8_t center_x, uint8_t center_y, uint8_t rad, SSD1306_Color color);
void SSD1306_FillRect1(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, SSD1306_Color color);
void SSD1306_FillRect2(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height, SSD1306_Color color);
void SSD1306_DrawRectangle1(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, SSD1306_Color color);
void SSD1306_DrawRectangle2(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height, SSD1306_Color color);
void SSD1306_DrawLine(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, SSD1306_Color color);
void SSD1306_PutString(uint8_t x, uint8_t y, char* c, MatrixFont font, SSD1306_Color color);
void SSD1306_PutChar(uint8_t x, uint8_t y, char c, MatrixFont font, SSD1306_Color color);
void SSD1306_DrawBitMap(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t width, uint8_t height, SSD1306_Color color);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_Color color);
void SSD1306_ClearScreen();
void SSD1306_FullScreen();
void SSD1306_UpdateScreen();

//Display Function
void SSD1306_SetContrast(uint8_t val);
void SSD1306_TurnOn(SSD1306_State state);
void SSD1306_InvertColor(SSD1306_State state);


#endif //SSD1306_IIC_H

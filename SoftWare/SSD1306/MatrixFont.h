//
// Created by AQin on 2023/2/13.
//

#ifndef MATRIXFONT_H
#define MATRIXFONT_H
#include <stdint.h>

#define SSD1306_USE_6x8
#define SSD1306_USE_7x10
#define SSD1306_USE_11x18
#define SSD1306_USE_16x26

typedef struct
{
    uint8_t width;
    uint8_t height;
    const uint16_t* bitmap;
}MatrixFont;

#ifdef SSD1306_USE_6x8
extern MatrixFont MF_6x8;
#endif

#ifdef SSD1306_USE_7x10
extern MatrixFont MF_7x10;
#endif

#ifdef SSD1306_USE_11x18
extern MatrixFont MF_11x18;
#endif

#ifdef SSD1306_USE_16x26
extern MatrixFont MF_16x26;
#endif

#endif //MATRIXFONT_H

/*! ***************************************************************************
 *
 * \brief     SDD1306 driver
 * \file      ssd1306.h
 * \author    Hugo Arends
 * \date      April 2021
 *
 * \copyright 2021 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#ifndef SSD1306_H
#define SSD1306_H

#include "lpi2c0_controller_polling.h"
#include "fonts.h"
#include "bitmaps.h"

/// \name Definitions for SSD1306
/// \{

/*!
 * \brief Definition for the logic value of the SA0 pin of the SSD1306 display
 */
#define SSD1306_SA0           (0)

/*!
 * \brief Definition for the display dimensions
 */
#define SSD1306_WIDTH         (128)
#define SSD1306_HEIGHT        (64)
#define SSD1306_SIZE          (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

/*!
 * \brief Definition for the slave address
 */
#define SSD1306_SLAVE_ADDRESS (0x78 | SSD1306_SA0)

/// \}

/// Value for a pixel
typedef enum
{
    ON,  ///< Pixel is on
    OFF, ///< Pixel is off
}
pixel_value_t;

extern uint8_t ssd1306_framebuffer[SSD1306_SIZE];

// Funtion prototypes
void ssd1306_init(void);
void ssd1306_command(const uint8_t cmd);
void ssd1306_data(const uint8_t data);
void ssd1306_update(void);

void ssd1306_setfont(const char *f);
void ssd1306_setorientation(const uint8_t orientation);
void ssd1306_setinverse(const uint8_t inv);

void ssd1306_clearscreen(void);
void ssd1306_setcontrast(const uint8_t contrast);
void ssd1306_goto(const uint8_t new_x, const uint8_t new_y);
void ssd1306_setpixel(const uint8_t x, const uint8_t y, const pixel_value_t val);

void ssd1306_putchar(const char c);
void ssd1306_putstring(const uint8_t xs, const uint8_t ys, const char *str);

void ssd1306_drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ssd1306_drawbitmap(const unsigned char *bitmap);

void ssd1306_terminal(const char *str);

#endif // SSD1306_H

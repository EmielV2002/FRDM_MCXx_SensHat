/*! ***************************************************************************
 *
 * \brief     SDD1306 driver
 * \file      ssd1306.c
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
#include "ssd1306.h"

#include <string.h>

static void delay_us(uint32_t d)
{
    volatile uint32_t t;

    for(t=256*d; t>0; t--)
    {
        __asm("nop");
        __asm("nop");
    }
}

/*!
 * \brief Framebuffer
 *
 * The framebuffer holds all the data. The framebuffer is only written to the
 * Oled display when the function ssd1306_update() is called.
 */
uint8_t ssd1306_framebuffer[SSD1306_SIZE];

/*!
 * \brief Pointer to the selected font
 *
 * The available fonts are in fonts.h and fonts.c.
 *
 * Note that Monospaced_plain_10 is the default font and should not me removed
 * from fonts.c and/or fonts.h.
 */
static const char *font = Monospaced_plain_10;

/*!
 * \brief Local x value
 */
static uint8_t x = 0;

/*!
 * \brief Local y value
 */
static uint8_t y = 0;

/*!
 * \brief List of commands that will be send to the Oled display upon
 *        initialisation
 *
 * This list was created based on the flowchart given in the SSD1306 datasheet.
 * Refer to the SSD1306 datasheet for a description of all commands
 */
static const uint8_t lpi2c0_controller_init_commands[] =
{
    0xAE,             // Display OFF
    0x20, 0x00,       // Horizontal addressing mode
    0x21, 0x00, 0x7F, // Column Address start and end (DEFAULT)
    0x22, 0x00, 0x07, // Page address start and end (DEFAULT)
    0xA8, 0x3F,       // Multiplex Ratio 63 (DEFAULT)
    0xD3, 0x00,       // Display offset 0 (DEFAULT)
    0x40,             // Display start line 0 (DEFAULT)
    0xA0,             // Segment Re-map (DEFAULT)
    0xC0,             // COM output scan direction normal (DEFAULT)
    0xDA, 0x12,       // COM pins alternative configuration
    0x81, 0xFF,       // Contrast control 256
    0xA4,             // Entire display on
    0xA6,             // Set normal display
    0xD5, 0xF0,       // Display Clock divide ratio/oscillator frequency
    0xDB, 0x20,       // V COMH deselect level ~0.77 x Vcc (DEFAULT)
    0x8D, 0x14,       // Enable charge pump
    0xAF,             // Display ON
};

/*!
 * \brief Sends a sequence of initialisation commands to the Oled display
 *
 * Refer to the SSD1306 datasheet for a description of all possible commands
 */
void ssd1306_init(void)
{
    // Clear the framebuffer
    for(uint32_t i=0; i<SSD1306_SIZE; ++i)
    {
        ssd1306_framebuffer[i] = 0;
    }

    // Initialize the KL25Z I2C peripheral
    lpi2c0_controller_init();

    // Initialize the SSD1306
    lpi2c0_write_cmd(SSD1306_SLAVE_ADDRESS, lpi2c0_controller_init_commands, 
        sizeof(lpi2c0_controller_init_commands));
}

/*!
 * \brief Sends a command to the Oled display
 *
 * Refer to the SSD1306 datasheet for a description of all possible commands
 *
 * \param[in]  cmd  Command
 */
void ssd1306_command(const uint8_t cmd)
{
    if(!lpi2c0_write_cmd(SSD1306_SLAVE_ADDRESS, &cmd, 1))
    {
        // Try to reinitialise the display if writing the command failed
        lpi2c0_controller_init();
        return;
    }
}

/*!
 * \brief Sends a data byte to the Oled display
 *
 * \param[in]  data  Data
 */
void ssd1306_data(const uint8_t data)
{
    if(!lpi2c0_write_data(SSD1306_SLAVE_ADDRESS, &data, 1))
    {
        // Try to reinitialise the display if writing the data failed
        lpi2c0_controller_init();
        return;
    }
}

/*!
 * \brief Sends the framebuffer to the Oled display
 *
 * The column and page addresses are set to the inital position.
 * Then the framebuffer is transferred.
 *
 * The total number of bytes to transfer is equal to:
 * - address byte + 6 command bytes
 * - address byte + (SSD1306_WIDTH * SSD1306_HEIGHT) / 8 data bytes
 *
 * The transmission of a single byte takes 1/375000 * 9 = 24 us
 *
 * Example for 128 x 64 display:
 * \n
 * 24 us * 1032 bytes = 24.768 ms is the total theoretical minimum time it takes
 * to send the framebuffer to the Oled display. Measurements show that it
 * actually takes approximately 28 ms.
 *
 * Notice that this is a non-blocking transfer.
 */
void ssd1306_update(void)
{
    uint8_t data[] =
    {
        0x21, 0x00, 0x7F, // Column Address start and end (DEFAULT)
        0x22, 0x00, 0x07, // Page address start and end (DEFAULT)
    };


    if(!lpi2c0_write_cmd(SSD1306_SLAVE_ADDRESS, data, sizeof(data)))
    {
        // Try to reinitialise the display if writing the command failed
        lpi2c0_controller_init();
        return;
    }

    // The display requires an idle time of 1.3 us (see table 13-6 in the
    // datasheet) between I2C transfers.
    delay_us(2);

    // Write the framebuffer to the device
    if(!lpi2c0_write_data(SSD1306_SLAVE_ADDRESS,
        ssd1306_framebuffer, sizeof(ssd1306_framebuffer)))
    {
        // Try to reinitialise the display if writing the data failed
        lpi2c0_controller_init();
        return;
    }
}

/*!
 * \brief Sets the font
 *
 * Changing a font doesn't change what is already visible on the Oled display.
 * This font will be used for writing new characters.
 * Fonts should be located in the files fonts.c and fonts.h.
 *
 * \param[in]  f  A pointer to a font
 */
void ssd1306_setfont(const char *f)
{
    font = f;
}

/*!
 * \brief Sets the display's orientation
 *
 * This function either sets default display orientation or flips the display
 * both horizontally and vertically.
 *
 * If \p orientation = 0: default display orientation
 *
 * If \p orientation > 0: flipped both horizontally and vertically
 *
 * \param[in]  orientation  Display orientation
 */
void ssd1306_setorientation(const uint8_t orientation)
{
    // The display requires an idle time before updating the orientiation
    delay_us(2);

    uint8_t data[2];

    if(orientation)
    {
        data[0] = 0xA1;
        data[1] = 0xC8;
    }
    else
    {
        data[0] = 0xA0;
        data[1] = 0xC0;
    }

    if(!lpi2c0_write_cmd(SSD1306_SLAVE_ADDRESS, data, sizeof(data)))
    {
        // Try to reinitialise the display if writing the command failed
        lpi2c0_controller_init();
        return;
    }
}

/*!
 * \brief Sets the display's inverse mode
 *
 * This function enables or disables the display's inverse mode
 *
 * If \p inv = 0: inverse mode disabled
 *
 * If \p inv > 0: inverse mode enabled
 *
 * \param[in]  inv  inverse mode
 */
void ssd1306_setinverse(const uint8_t inv)
{
    uint8_t data = (inv) ? 0xA7 : 0xA6;

    ssd1306_command(data);
}

/*!
 * \brief Clears the display
 *
 * Clears all data in the framebuffer.
 * Call the function ssd1306_update() to actually show the result.
 */
void ssd1306_clearscreen(void)
{
    memset(ssd1306_framebuffer, 0x00, sizeof(ssd1306_framebuffer));
}

/*!
 * \brief Sets the display's contrast
 *
 * This function sets the contrast between 0 and 255
 *
 * \param[in]  contrast  Contrast value
 */
void ssd1306_setcontrast(const uint8_t contrast)
{
    uint8_t data[2] =
    {
        0x81, 0x00,
    };

    data[1] = contrast;

    if(!lpi2c0_write_cmd(SSD1306_SLAVE_ADDRESS, data, sizeof(data)))
    {
        // Try to reinitialise the display if writing the command failed
        lpi2c0_controller_init();
        return;
    }
}

/*!
 * \brief Sets x and y
 *
 * This function updates the current (x,y) value
 *
 * \param[in]  new_x  New value for x
 * \param[in]  new_y  New value for y
 */
void ssd1306_goto(const uint8_t new_x, const uint8_t new_y)
{
	if((new_x >= SSD1306_WIDTH) || (new_y >= SSD1306_HEIGHT))
    {
		return;
	}

    x = new_x;
    y = new_y;
}

/*!
 * \brief Sets the pixel at (x,y) to val
 *
 * This function sets the value of the pixel at location (x,y).
 * Call the function ssd1306_update() to actually show the result.
 *
 * \param[in]  nx   x-value
 * \param[in]  ny   y-value
 * \param[in]  val  Pixel value
 */
void ssd1306_setpixel(const uint8_t nx, const uint8_t ny, const pixel_value_t val)
{
	if(val == ON)
    {
		ssd1306_framebuffer[nx + (ny / 8) * SSD1306_WIDTH] |= 1 << (ny % 8);
	}
    else
    {
		ssd1306_framebuffer[nx + (ny / 8) * SSD1306_WIDTH] &= ~(1 << (ny % 8));
	}
}

/*!
 * \brief Displays a character at the current (x,y) position
 *
 * Writes a character into the framebuffer, using the selected font.
 * The (x,y) location is the top-left location of the character.
 * After writing a char to the framebuffer, y is not updated, only x.
 * Call the function ssd1306_update() to actually show the result.
 *
 * \param[in]  c  Character to display
 */
void ssd1306_putchar(const char c)
{
    // Get the first four parameters from the font
  //uint8_t font_width = font[0];
    uint8_t font_height = font[1];
    uint8_t font_firstchar = font[2];
    uint8_t font_numchars = font[3];

    // Calculate the index in the jump table
    uint32_t index = 4 + ((c - font_firstchar) * 4);

    // Read the data from the jump table
    uint8_t offset1 = font[index];
    uint8_t offset2 = font[index + 1];
    uint8_t n_bytes = font[index + 2];
    uint8_t char_width = font[index + 3];

    // Calculate the index in the data table
    index = 4 + (font_numchars * 4) + (256UL * offset1) + offset2;

    // Calculate the number of bytes for each column
    uint8_t bytes_per_col = (font_height / 8);
    if((font_height % 8) > 0)
    {
        bytes_per_col++;
    }

    // Calculate the total number of bytes for this character
    uint8_t bytes_total = bytes_per_col * char_width;

    // Temporary variable for y position
    uint8_t y_tmp = y;

    // Variable counting the character height in pixels
    uint8_t height = 0;

    // Loop all bytes inb a character
    for(uint32_t n=0; n<bytes_total; n++)
    {
        // Initially a byte is cleared
        char data = 0;

        // Is there data available in the character table for this byte?
        if(n < n_bytes)
        {
            data = font[index + n];
        }

        // Next column?
        if((n % bytes_per_col) == 0)
        {
            y_tmp=y;
            x++;

            // Stop if the x value is outside screen boundaries
            if(x >= SSD1306_WIDTH)
            {
                return;
            }

            height = 0;
        }

        // Loop all bits in the byte and update the framebuffer
        for(uint8_t mask = 1; mask != 0; mask <<= 1)
        {
            pixel_value_t val = (data & mask) ? ON : OFF;
            ssd1306_setpixel(x,y_tmp,val);
            y_tmp++;

            // Stop if the y value is outside screen boundaries
            if(y_tmp >= SSD1306_HEIGHT)
            {
                break;
            }

            // Stop if the font height has been reached
            height++;
            if(height >= font_height)
            {
                break;
            }
        }
    }
}

/*!
 * \brief Displays a string of characters starting at position (xs,ys).
 *
 * Writes a string of character into the framebuffer, using the selected font.
 * After writing a char to the framebuffer, y is not updated, only x.
 * Call the function ssd1306_update() to actually show the result.
 *
 * A '\n' character moves the (x,y) position to the next line, taking the
 * current selected font height into account.
 *
 * A '\r' character is ignored.
 *
 * \param[in]  xs   x-value of the string
 * \param[in]  ys   y-value of the string
 * \param[in]  str  '\0' terminated string
 */
void ssd1306_putstring(const uint8_t xs, const uint8_t ys, const char *str)
{
    uint8_t delta = 0;

    ssd1306_goto(xs,ys);

    uint32_t i=0;
    while(str[i] != '\0')
    {
        if(str[i] == '\n')
        {
            // Go to a new line
            // Set the original x value and increment the y value by the font
            // height
            delta += font[1];
            ssd1306_goto(xs,ys+delta);
        }
        else if(str[i] == '\r')
        {
            // Ignore
        }
        else
        {
            ssd1306_putchar(str[i]);
        }

        i++;
    }
}

/*!
 * \brief Emulates a mini terminal
 *
 * Writes a string of characters into the lowest line of the framebuffer, using
 * the selected font. The function updates the Oled display by calling the
 * function ssd1306_update().
 *
 * A '\n' character scrolls all lines one line up, taking the current selected
 * font height into account and clears the bottom line.
 *
 * A '\r' character moves the x position to the beginning of the line. This
 * means that previous written characters will be overwritten.
 *
 * \param[in]  str  '\0' terminated string
 */
void ssd1306_terminal(const char *str)
{
    char offset = font[1];

    y = SSD1306_HEIGHT-offset-1;

    uint32_t i=0;
    while(str[i] != '\0')
    {
        if(str[i] == '\n')
        {
            // Move the previous characters up
            for(uint32_t yn = offset; yn < SSD1306_HEIGHT; yn++)
            {
                for(uint32_t xn = 0; xn < SSD1306_WIDTH; xn++)
                {
                    pixel_value_t val = OFF;
                    if(ssd1306_framebuffer[xn + (yn / 8) * SSD1306_WIDTH] & (1 << (yn % 8)))
                    {
                        val = ON;
                    }

                    ssd1306_setpixel((uint8_t)xn, (uint8_t)(yn-offset), val);
                }
            }

            // Clear bottom
            for(uint32_t yn = SSD1306_HEIGHT-offset-1; yn < SSD1306_HEIGHT; yn++)
            {
                for(uint32_t xn = 0; xn < SSD1306_WIDTH; xn++)
                {
                    pixel_value_t val = OFF;
                    ssd1306_setpixel((uint8_t)xn, (uint8_t)yn, val);
                }
            }

            ssd1306_goto(0,SSD1306_HEIGHT-offset-1);
        }
        else if(str[i] == '\r')
        {
            ssd1306_goto(0,SSD1306_HEIGHT-offset-1);
        }
        else
        {
            ssd1306_putchar(str[i]);
        }

        i++;
    }

//    ssd1306_update();
}

/*!
 * \brief Draws a line from (x0,y0) tot (x1,y1)
 *
 * Draws a line in the framebuffer using Bresenham's line algorithm. The
 * implementation is based on the following information:
 * https://csustan.csustan.edu/~tom/Lecture-Notes/Graphics/Bresenham-Line/Bresenham-Line.pdf
 *
 * Call the function ssd1306_update() to actually show the result.
 *
 * \param[in]  x0  x-value of the start point
 * \param[in]  y0  y-value of the start point
 * \param[in]  x1  x-value of the end point
 * \param[in]  y1  y-value of the end point
 */
void ssd1306_drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    int stepx, stepy;

    if (dy < 0) { dy = -dy; stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx; stepx = -1; } else { stepx = 1; }

    dy <<= 1;
    dx <<= 1;

    if((x0 < SSD1306_WIDTH) && (y0 < SSD1306_HEIGHT))
    {
        ssd1306_setpixel(x0, y0, ON);
    }

    if (dx > dy)
    {
        int fraction = dy - (dx >> 1);

        while (x0 != x1)
        {
            x0 += stepx;
            if (fraction >= 0)
            {
                y0 += stepy;
                fraction -= dx;
            }

            fraction += dy;
            if((x0 < SSD1306_WIDTH) && (y0 < SSD1306_HEIGHT))
            {
                ssd1306_setpixel(x0, y0, ON);
            }
        }

    }
    else
    {
        int fraction = dx - (dy >> 1);

        while (y0 != y1)
        {
            if (fraction >= 0)
            {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;

            fraction += dx;
            if((x0 < SSD1306_WIDTH) && (y0 < SSD1306_HEIGHT))
            {
                ssd1306_setpixel(x0, y0, ON);
            }
        }
    }
}

/*!
 * \brief Draws a bitmap
 *
 * Copies a bitmap to the framebuffer.
 * Call the function ssd1306_update() to actually show the result.
 * Bitmaps should be located in the files bitmaps.c and bitmaps.h.
 *
 * \param[in]  bitmap  A pointer to a bitmap
 */
void ssd1306_drawbitmap(const unsigned char *bitmap)
{
    memcpy(ssd1306_framebuffer, bitmap, sizeof(ssd1306_framebuffer));
}

int ssd1306_main(void)
{
  // Set initial message
  ssd1306_setorientation(1);
  ssd1306_setfont(Monospaced_plain_10);
  ssd1306_clearscreen();
  ssd1306_update();                
            
  // Delay
  for(volatile int i=0; i<1000000; i++)
  {}
  while(1)
  {
    ssd1306_drawbitmap(nxp_logo);
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(53, 42, "MCX");
    ssd1306_putstring(38, 53, "A Series");
    ssd1306_update();       

    // Delay
    for(volatile int i=0; i<10000000; i++)
    {}

    ssd1306_drawbitmap(bitmap1);
    ssd1306_setfont(Monospaced_bold_24);
    ssd1306_putstring(54, 16, "HAN_");
    ssd1306_update();       

    // Delay
    for(volatile int i=0; i<10000000; i++)
    {}
  }
}

/*! ***************************************************************************
 *
 * \brief     Definitions of bitmaps
 * \file      bitmaps.h
 * \author    Hugo Arends
 * \date      April 2021
 *
 * \remark    Add a new bitmap as follows
 * 
 * 1. Create a 128 x 64 bitmap, for example with this great tool:
 *    http://en.radzio.dxp.pl/bitmap_converter/
 * 2. Add all the generated code in the file bitmaps.c
 * 3. Add an external declaration to the file bitmaps.h
 * 4. Draw the bitmap by passing the pointer to the function 
 *    ssd1306_drawbitmap()
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
#ifndef BITMAPS_H_
#define BITMAPS_H_

extern const unsigned char bitmap1[];
extern const unsigned char bitmap2[];
extern const unsigned char nxp_logo[];

#endif // BITMAPS_H_

/*! ***************************************************************************
 *
 * \brief     Definitions of fonts
 * \file      fonts.h
 * \author    Hugo Arends
 * \date      April 2021
 *
 * \remark    Add a new font as follows
 * 
 * 1. Copy a library version >=3.0.0 font from the following great website: 
 *    http://oleddisplay.squix.ch/
 * 2. Add all the generated code in the file fonts.c
 * 3. Remove 'PROGMEM' from the declaration
 * 4. Add an external declaration to the file fonts.h
 * 5. Set the font by passing the pointer to the function ssd1306_setfont()
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
#ifndef FONTS_H_
#define FONTS_H_

extern const char Monospaced_plain_10[];
extern const char Dialog_plain_12[];
extern const char Monospaced_bold_24[];

#endif // FONTS_H_

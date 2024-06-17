/*! ***************************************************************************
 *
 * \brief     Low level driver for the FT24C08A 
 * \file      ft24c08a.h
 * \author    Hugo Arends
 * \date      April 2024
 *
 * \copyright 2024 HAN University of Applied Sciences. All Rights Reserved.
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
 ******************************************************************************/
#ifndef FT24C08A_H
#define FT24C08A_H

#include <MCXA153.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Shared type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Shared variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Shared function prototypes
// -----------------------------------------------------------------------------
void ft24c08a_init(void);
void ft24c08a_wp(const bool enable);

// This driver support 8-bit data word addresses only. This means random word
// addressing only works for the bytes at addresses 0 to 255.
//
// When reading/writing multiple bytes, a.k.a. page read/write in the datasheet,
// the maximum allowed len is 16. If the address is not a multiple of 16, the
// read/write wraps around and does NOT continue to the next page!
void ft24c08a_read(uint8_t address, uint8_t *data, uint8_t len);
void ft24c08a_write(uint8_t address, uint8_t *data, uint8_t len);

#endif // FT24C08A_H

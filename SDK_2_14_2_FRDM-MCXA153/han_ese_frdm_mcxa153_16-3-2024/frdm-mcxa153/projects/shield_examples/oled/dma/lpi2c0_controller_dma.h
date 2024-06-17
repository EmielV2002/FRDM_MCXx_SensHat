/*! ***************************************************************************
 *
 * \brief     Low-Power I2C controller DMA
 * \file      lpi2c0_controller_dma.c
 * \author    Hugo Arends
 * \date      March 2024
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
 *****************************************************************************/
#ifndef LPI2C0_CONTROLLER_DMA_H
#define LPI2C0_CONTROLLER_DMA_H

#include <MCXA153.h>
#include <stdbool.h>

void lpi2c0_controller_init(void);
bool lpi2c0_write_cmd(const uint8_t address, const uint8_t cmd[], const uint32_t n);
bool lpi2c0_write_data(const uint8_t address, const uint8_t data[], const uint32_t n);

#endif // LPI2C0_CONTROLLER_DMA_H

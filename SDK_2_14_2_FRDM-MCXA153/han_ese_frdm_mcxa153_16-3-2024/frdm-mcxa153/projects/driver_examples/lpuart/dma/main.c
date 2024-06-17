/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2024
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
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
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>
#include "gpio_output.h"
#include "lpuart0_dma.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static uint8_t data[8] = {0};
static uint8_t str[128] = {0};

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    gpio_output_init();
    lpuart0_dma_init(9600);

    // Show welcome message
    sprintf((char *)str, "\r\n\r\nLPUART0 DMA - %s\r\nBuild %s %s\r\n",
        TARGETSTR,  __DATE__, __TIME__);
    lpuart0_dma_write(str, (uint16_t)strlen((char *)str));

    // Initialize data
    for(uint32_t i=0; i<sizeof(data); i++)
    {
        data[i] = 'A';
    }

    // Write data by using DMA
    lpuart0_dma_write(data, sizeof(data));

    // Show message
    sprintf((char *)str, "\r\nEnter 8 characters. They will override the items "
        "in the data buffer.\r\n");
    lpuart0_dma_write(str, (uint16_t)strlen((char *)str));

    // Read data from the terminal application by using DMA
    lpuart0_dma_read(data, 8);

    // Wait until all data is read
    while(!lpuart0_dma_read_done())
    {}

    // Write data by using DMA
    lpuart0_dma_write(data, sizeof(data));

    // Done
    sprintf((char *)str, "\r\nDone!\r\n");
    lpuart0_dma_write(str, (uint16_t)strlen((char *)str));

    while(1)
    {}
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

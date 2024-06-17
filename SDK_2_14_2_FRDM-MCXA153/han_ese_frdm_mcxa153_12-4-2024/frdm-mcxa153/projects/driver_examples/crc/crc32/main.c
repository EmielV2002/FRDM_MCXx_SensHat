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
#include <stdbool.h>
#include "crc0.h"
#include "gpio_output.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    crc0_init();
    gpio_output_init();
    
    bool error = false;
    uint32_t checksum = 0;
    
    // Checksums for verification calculated with https://crccalc.com/
    
    // -------------------------------------------------------------------------

    // Prepare data
    uint32_t data0[] = {0x00000000};
    
    // Calculate checksum
    checksum = crc0_calculate(data0, sizeof(data0)/sizeof(uint32_t));
    
    // Verify the result
    error = (checksum != 0x2144DF1C) ? true : error;
    
    // -------------------------------------------------------------------------

    // Prepare data
    uint32_t data1[] = {0x12345678, 0xAA55AA55};
    
    // Calculate checksum
    checksum = crc0_calculate(data1, sizeof(data1)/sizeof(uint32_t));
    
    // Verify the result
    error = (checksum != 0x13ADCCB8UL) ? true : error;

    // -------------------------------------------------------------------------

    // Prepare data
    uint32_t data2[] = {0x00001111, 0x22223333, 0x44445555, 0x66667777, 
                        0x88889999, 0xAAAABBBB, 0xCCCCDDDD, 0xEEEEFFFF,
                        0x00001111, 0x22223333, 0x44445555, 0x66667777, 
                        0x88889999, 0xAAAABBBB, 0xCCCCDDDD, 0xEEEEFFFF};
    
    // Calculate checksum
    checksum = crc0_calculate(data2, sizeof(data2)/sizeof(uint32_t));
    
    // Verify the result
    error = (checksum != 0xED561F34UL) ? true : error;
    
    // -------------------------------------------------------------------------

    while(1)
    {
        if(error)
        {
            // Red RGB LED on
            GPIO3->PCOR = (1<<12);
        }
        else
        {
            // Green RGB LED on
            GPIO3->PCOR = (1<<13);    
        }
        
        // Delay
        for(volatile int i=0; i<20000; i++)
        {}
        
        // Both RGB LEDs off
        GPIO3->PSOR = (1<<12) | (1<<13);       

        // Delay
        for(volatile int i=0; i<2000000; i++)
        {}
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

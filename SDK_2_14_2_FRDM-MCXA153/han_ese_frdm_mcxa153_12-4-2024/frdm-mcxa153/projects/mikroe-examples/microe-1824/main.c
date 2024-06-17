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
#include "ec12d1564402.h"
#include "hc595.h"
#include "lpuart0_interrupt.h"

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
static volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;
static const uint32_t interval_ms = 100;

static uint16_t leds = 0;
static uint16_t tmp = 0;
static int32_t cnt = 0;
static int32_t previous_cnt = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    lpuart0_init(115200);
    ec12d1564402_init();
    hc595_init();
    
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);   
    
    printf("Microe 1824: Rotary B click - example project");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    
    printf("cnt = %d\r\n", cnt);

    leds = 1;
    hc595_write((uint8_t *)&leds, 2);

    // Empty receive fifo
    hc595_read((uint8_t *)&tmp, 2);   
    
    while(1)
    {
        uint32_t current_ms = ms;
        
        // --------------------------------------------------------------------
        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;
            
            // Next LED
            cnt = ec12d1564402_pulses();
            
            if(cnt != previous_cnt)
            {
                previous_cnt = cnt;
                
                printf("cnt = %d\r\n", cnt);
            
                if(cnt < 0)
                {
                    cnt = (-cnt) % 16;
                    leds = cnt == 0 ? 1 : 0x8000 >> (cnt-1);
                }
                else
                {
                    cnt = cnt % 16;
                    leds = (uint16_t)(0x0001 << cnt);
                }
                
                hc595_write((uint8_t *)&leds, 2);       
                
                // Empty receive fifo
                hc595_read((uint8_t *)&tmp, 2);       
            }
        }
        
        // --------------------------------------------------------------------
        if(ec12d1564402_sw_pressed())
        {
            printf("sw pressed\r\n");
        }
        
        // --------------------------------------------------------------------
        if(ec12d1564402_sw_released())
        {
            printf("sw released\r\n");
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;    
}

/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      May 2024
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
#include "encoder.h"
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

static uint8_t leds = 0;
static int32_t cnt = 0;
static int32_t previous_cnt = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    lpuart0_init(9600);
    encoder_init();
    hc595_init();
    
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);   
    
    printf("Encoder and Shift register - example project");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    
    printf("cnt = %d\r\n", cnt);

    leds = 0x01;
    hc595_write(leds);
   
    while(1)
    {
        uint32_t current_ms = ms;
        
        // --------------------------------------------------------------------
        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;
            
            // Next LED
            cnt = encoder_pulses();
            
            if(cnt != previous_cnt)
            {
                previous_cnt = cnt;
                
                printf("cnt = %d\r\n", cnt);
            
                if(cnt < 0)
                {
                    cnt = (-cnt) % 8;
                    leds = cnt == 0 ? 1 : 0x80 >> (cnt-1);
                }
                else
                {
                    cnt = cnt % 8;
                    leds = (uint8_t)(0x01 << cnt);
                }
                
                hc595_write(leds);             
            }
        }
        
        // --------------------------------------------------------------------
        if(encoder_sw_pressed())
        {
            printf("sw pressed\r\n");
        }
        
        // --------------------------------------------------------------------
        if(encoder_sw_released())
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

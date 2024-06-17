/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      March 2024
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
#include "buzzer.h"
#include "gpio_input_interrupt.h"
#include "gpio_output.h"
#include "lpuart0_interrupt.h"
#include "neopixel_flexpwm.h"

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
static const uint32_t interval_ms = 50;
static volatile int32_t cnt = 0;
static volatile uint32_t color = 0;
static volatile uint32_t direction = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    gpio_output_init();
    gpio_input_init();
    neopixel_init();
    buzzer_init();
    lpuart0_init(9600);

    // 96 MHz FIRC clock selected
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);
        
    // Initialize SysTick to generate an interrupt every 1ms
    SysTick_Config(96000);    
    NVIC_SetPriority(SysTick_IRQn, 1);
    
    // Prevents the first RGB LED from blinking too bright
    while(ms < 200)
    {}
    
    printf("RGB Clock - FlexPWM");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    
    rgb_t pixels[N_NEOPIXELS] = {0};    
    neopixel_update(pixels);
    while(ms < 300)
    {}

    const uint8_t brightness = 2;    

    while(1)
    {        
        uint32_t current_ms = ms;
        
        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;
            
            buzzer_off();
            
            pixels[cnt] = (rgb_t){0,0,0};

            // Next pixel
            if(direction == 0)
            {
                cnt = cnt + 1;
                
                if(cnt >= N_NEOPIXELS)
                {
                    buzzer_on(500 + (200 * (uint16_t)color));

                    cnt = 0;
                    color = (color + 1) % 3;
                }
            }
            else
            {
                cnt = cnt - 1;
                
                if(cnt < 0)
                {
                    cnt = N_NEOPIXELS;
                    color = (color + 1) % 3;
                }
            }
            
            pixels[cnt] = (rgb_t){color == 0 ? brightness : 0,
                                  color == 1 ? brightness : 0,
                                  color == 2 ? brightness : 0};

            neopixel_update(pixels);
        }
    
    	if(lpuart0_rxcnt() > 0)
    	{
			int c = getchar();

			if((c == 'd') || (c == 'D'))
			{
				direction = !direction;
                printf("%c - direction changed\r\n", c);
			}
    	}
        
        if(gpio_input_sw3_pressed())
        {
            direction = !direction;
            printf("SW3 - direction changed\r\n");
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

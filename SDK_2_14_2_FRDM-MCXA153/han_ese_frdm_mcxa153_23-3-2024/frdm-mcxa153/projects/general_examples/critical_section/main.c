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
#include "gpio_input_interrupt.h"
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
static uint8_t cnt = 0;
static char global_buffer[16];

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    gpio_input_init();
    lpuart0_init(9600);
    
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);   
    
    printf("Critical section");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    
    printf("\r\n");
    printf("Press SW3 to print the buffer. The printed line should\r\n" \
        "display the same number between 0 and 9. However, the critical\r\n" \
        "section (copying the global buffer in main to a local buffer \r\n" \
        "before printing) is not protected from preemption. This means \r\n" \
        "that if the timing is right, the printed line might show \r\n" \
        "different numbers.\r\n");

    while(1)
    {
        // Switch pressed?
        if(gpio_input_sw3_pressed())
        {
            // Add one for string terminator '\0'
            char local_buffer[sizeof(global_buffer) + 1];
            
            // Copy the global buffer
            for(uint32_t i=0; i<sizeof(global_buffer); i++)
            {
                local_buffer[i] = global_buffer[i];
                
                // Artificial delay to increase the chance of interrupting this
                // critical section
                for(uint32_t d=0; d<20000; d++)
                {}                
            }
            
            // Terminate the buffer and print it
            local_buffer[sizeof(global_buffer)] = '\0';
            printf(local_buffer);
            printf("\r\n");
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
    
    // Every second, increment the counter and fill global buffer with it's
    // ASCII representation
    if((ms % 1000) == 0)
    {
        cnt = (cnt + 1) % 10;
        
        for(uint32_t i=0; i<sizeof(global_buffer); i++)
        {
            global_buffer[i] = (char)('0' + cnt);
        }        
    }
}

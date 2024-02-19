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
#include "general_purpose_output.h"
#include "usb_device.h"

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

static volatile unsigned ms = 0;

//static void putstr(char* s)
//{
//    while (*s)
//    {
//        fifo_push(&usb_tx, *s++);
//    }
//}

int main(void)
{
    unsigned start_time = 0;
    uint8_t count = 0;
    uint8_t c;

    // Generate an interrupt every 1 ms
    SysTick_Config(48000);    

    gpo_init();
    usb_device_init();
    
    // Enable interrupts
    __enable_irq();    
    
    while(1)
    {
        // Wait for interrupt
        __WFI();

        // Echo all data from RX fifo into TX fifo
        while(fifo_pop(&usb_rx, &c))
        {
            fifo_push(&usb_tx, c);
        }

        // Insert a capital letter every 250ms
        if((ms - start_time) > 250)
        {
            start_time = ms;
            fifo_push(&usb_tx, 'A' + count);
            count = (count == 25) ? 0 : count+1;
        }
    }
}


/*
 * Hooks and Interrupts
 */

/**
 * Hook is called when receiving a message packet.
 * @param data pointer to 63 bytes containing the message
 */
void usb_hook_message_packet(volatile uint8_t* data)
{
    if(data[0] == 1)
    {
        // Blue LED on
        GPIO3->PCOR = (1<<0);
    }
    else 
    {
        // Blue LED off
        GPIO3->PSOR = (1<<0);
    }
}

/**
 * Hook is called to control the RX LED
 * @param on true if LED should be on, false otherwise
 */
//void usb_hook_led_rx(bool on)
//{
//    if(on) 
//    {
//        // Red LED on
//        GPIO3->PCOR = (1<<12);
//    }
//    else 
//    {
//        // Red LED off
//        GPIO3->PSOR = (1<<12);
//    }
//}

/**
 * Hook is called to control the TX LED
 * @param on true if LED should be on, false otherwise
 */
//void usb_hook_led_tx(bool on)
//{
//    if (on)
//    {
//        // Green LED on
//        GPIO3->PCOR = (1<<13);
//    }
//    else 
//    {
//        // Green LED off
//        GPIO3->PSOR = (1<<13);
//    }
//}

void SysTick_Handler(void)
{
    ++ms;
}

/*! ***************************************************************************
 *
 * \brief     General purpose input/output - input interrupt
 * \file      gpio_input_interrupt.c
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
#include "gpio_input_interrupt.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t cnt = 0;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void gpio_input_init(void)
{
    // From section 14.3 Functional description (NXP, 2024)
    //
    // Before a module can be used, its clocks must be enabled (CC != 00) and it
    // must be released from reset (MRCC_GLB_RST [peripherals name] = 1). If a
    // module is not released from reset (MRCC_GLB_RST [peripherals name] = 0),
    // an attempt to access a register within that module is terminated with a
    // bus error.

    // Enable modules and leave others unchanged
    // PORT1: [1] = Peripheral clock is enabled
    // GPIO1: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO1(1);

    // Release modules from reset and leave others unchanged
    // PORT1: [1] = Peripheral is released from reset
    // GPIO1: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO1(1);

    // From section 11.4 Initialization (NXP, 2024)
    //
    // 1. Initialize the pin functions:
    //    -  Initialize single pin functions by writing appropriate values to
    //       PCRn
    //    -  Initialize multiple pins (up to 16) with the same configuration by
    //       writing appropriate values to Global Pin Control Low (GPCLR) or
    //       Global Pin Control High (GPCHR).
    // 2. Lock the configuration for a given pin, by writing 1 to PCRn [LK], so
    //    that it cannot be changed until the next reset.

    // 1. & 2.
    //
    // Configure pin P1_7
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Input Buffer Enable
    // MUX: [0000] = Alternative 0 (GPIO)
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT1->PCR[7] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);
    
    // From section 12.5 Initialization (NXP, 2024)
    //
    // 1. n.a.
    // 2. Initialize the interrupt function by writing to Interrupt Control a 
    //    (ICR0 - ICR31) for the corresponding pins and desired configuration.
    //    If the pin is previously used for a different function, first write
    //    0100_0000h to Interrupt Control a (ICR0 - ICR31) to disable the
    //    previous function and clear the flag.

    // 2.
    //
    // ISF: [1] = Clear the flag
    // IRQC : [1010] = ISF and interrupt on falling edge
    GPIO1->ICR[7] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);
    
	// Enable GPIO1 interrupts
	NVIC_SetPriority(GPIO1_IRQn, 3);    
    NVIC_ClearPendingIRQ(GPIO1_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);

    // Globally enable interrupts
    __enable_irq();
}

// Interrupt handler
// Refer to the file startup_MCXA153.s for finding the function name.
void GPIO1_IRQHandler(void)
{
	// Clear the interrupt
	NVIC_ClearPendingIRQ(GPIO1_IRQn);

    // Interrupt handler triggered by P1_7?
    if((GPIO1->ISFR[0] & GPIO_ISFR_ISF7(1)) != 0)
    {
        // Clear the flag
        GPIO1->ISFR[0] = GPIO_ISFR_ISF7(1);
        
        // Handle the event
        cnt++;
    }
}

bool gpio_input_sw3_pressed(void)
{
    // Any presses detected since the last time checked?
    if(cnt > 0)
    {
        // Decrement presses counter
        cnt--;

        return true;
    }
    else
    {
        return false;
    }
}

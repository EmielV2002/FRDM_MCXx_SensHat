/*! ***************************************************************************
 *
 * \brief     Low level driver for the FXLS8974CF
 * \file      fxls8974cf.c
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
#include "fxls8974cf.h"
#include "lpi2c0_controller_interrupt.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
volatile bool fxls8974cf_int0 = false;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void fxls8974cf_init(void)
{
    // From section 14.3 Functional description (NXP, 2024)
    //
    // Before a module can be used, its clocks must be enabled (CC != 00) and it
    // must be released from reset (MRCC_GLB_RST [peripherals name] = 1). If a
    // module is not released from reset (MRCC_GLB_RST [peripherals name] = 0),
    // an attempt to access a register within that module is terminated with a
    // bus error.

    // Enable modules and leave others unchanged
    // PORT2: [1] = Peripheral clock is enabled
    // GPIO2: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);

    // Release modules from reset and leave others unchanged
    // PORT2: [1] = Peripheral is released from reset
    // GPIO2: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO2(1);

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
    // Configure pin P2_5
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
    PORT2->PCR[5] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);
    
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
    // IRQC : [1010] = ISF and interrupt on rising edge
    GPIO2->ICR[5] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1001);
    
    // Enable GPIO1 interrupts
    NVIC_SetPriority(GPIO2_IRQn, 3);    
    NVIC_ClearPendingIRQ(GPIO2_IRQn);
    NVIC_EnableIRQ(GPIO2_IRQn);
    
    lpi2c0_controller_init();
}

void GPIO2_IRQHandler(void)
{
    // Clear the interrupt
    NVIC_ClearPendingIRQ(GPIO2_IRQn);

    // Interrupt handler triggered by P2_5?
    if((GPIO2->ISFR[0] & GPIO_ISFR_ISF5(1)) != 0)
    {
        // Clear the flag
        GPIO2->ISFR[0] = GPIO_ISFR_ISF5(1);
        
        // Handle the event
        fxls8974cf_int0 = true;
    }
}

void fxls8974cf_read(uint8_t reg, uint8_t *data, uint8_t len)
{
    lpi2c0_read(0b00011000, reg, data, len);
    
    // Wait while busy
    while(lpi2c0_busy())
    {}
    
    return;
}

void fxls8974cf_write(uint8_t reg, uint8_t *data, uint8_t len)
{
    lpi2c0_write(0b00011000, reg, data, len);
    
    // Wait while busy
    while(lpi2c0_busy())
    {}
    
    return;
}

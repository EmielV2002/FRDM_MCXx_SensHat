/*! ***************************************************************************
 *
 * \brief     Low level driver for the 74HC595
 * \file      hc595.c
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
#include "hc595.h"
#include "lpspi0_master.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile int32_t cnt = 0;
static volatile bool sw_pressed = false;
static volatile bool sw_released = false;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void ec12d1564402_init(void)
{
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

    // Configure pins P2_5
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Input Buffer Enable
    // MUX: [0000] = Alternative 0 (GPIO)
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [1] = Enables
    // PS:  [1] = Enables internal pullup resistor
    PORT2->PCR[5] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);

    // 1. n.a.
    // 2. Initialize the interrupt function by writing to Interrupt Control a
    //    (ICR0 - ICR31) for the corresponding pins and desired configuration.
    //    If the pin is previously used for a different function, first write
    //    0100_0000h to Interrupt Control a (ICR0 - ICR31) to disable the
    //    previous function and clear the flag.

    // 2.
    //
    // ISF: [1] = Clear the flag
    // IRQC : [1010] = ISF and interrupt on both edges
    GPIO2->ICR[5] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1011);

    // Enable GPIO2 interrupts
    NVIC_SetPriority(GPIO2_IRQn, 7);
    NVIC_ClearPendingIRQ(GPIO2_IRQn);
    NVIC_EnableIRQ(GPIO2_IRQn);


    // Enable modules and leave others unchanged
    // PORT3: [1] = Peripheral clock is enabled
    // GPIO3: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);

    // Release modules from reset and leave others unchanged
    // PORT3: [1] = Peripheral is released from reset
    // GPIO3: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    // Configure pins P3_12 and P3_30
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Input Buffer Enable
    // MUX: [0000] = Alternative 0 (GPIO)
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [1] = Enables
    // PS:  [1] = Enables internal pullup resistor
    PORT3->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);
    PORT3->PCR[30] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);

    // 1. n.a.
    // 2. Initialize the interrupt function by writing to Interrupt Control a
    //    (ICR0 - ICR31) for the corresponding pins and desired configuration.
    //    If the pin is previously used for a different function, first write
    //    0100_0000h to Interrupt Control a (ICR0 - ICR31) to disable the
    //    previous function and clear the flag.

    // 2.
    //
    // ISF: [1] = Clear the flag
    // IRQC : [1010] = ISF and interrupt on both edges
    GPIO3->ICR[12] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1011);

    // Enable GPIO3 interrupts
    NVIC_SetPriority(GPIO3_IRQn, 3);
    NVIC_ClearPendingIRQ(GPIO3_IRQn);
    NVIC_EnableIRQ(GPIO3_IRQn);
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

        // Falling edge?
        if((GPIO2->PDIR & (1<<5)) == 0)
        {
            sw_released = true;
        }
        else
        {
            sw_pressed = true;
        }
    }
}

void GPIO3_IRQHandler(void)
{
    // Clear the interrupt
    NVIC_ClearPendingIRQ(GPIO3_IRQn);

    // Interrupt handler triggered by P3_12?
    if((GPIO3->ISFR[0] & GPIO_ISFR_ISF12(1)) != 0)
    {
        // Clear the flag
        GPIO3->ISFR[0] = GPIO_ISFR_ISF12(1);

        // Falling edge?
        if((GPIO3->PDIR & (1<<12)) == 0)
        {
            if((GPIO3->PDIR & (1<<30)) == 0)
            {
                cnt--;
            }
            else
            {
                cnt++;
            }
        }
        else
        {
            if((GPIO3->PDIR & (1<<30)) == 0)
            {
                cnt++;
            }
            else
            {
                cnt--;
            }
        }
    }
}

int32_t ec12d1564402_pulses(void)
{
    return cnt;
}

bool ec12d1564402_sw_pressed(void)
{
    // Critical section start
    uint32_t m = __get_PRIMASK();
    __disable_irq();
    
    bool ret = sw_pressed;
    sw_pressed = false;
    
    // Critical section end
    __set_PRIMASK(m);
        
    return ret;
}

bool ec12d1564402_sw_released(void)
{
    // Critical section start
    uint32_t m = __get_PRIMASK();
    __disable_irq();

    bool ret = sw_released;
    sw_released = false;

    // Critical section end
    __set_PRIMASK(m);

    return ret;
}

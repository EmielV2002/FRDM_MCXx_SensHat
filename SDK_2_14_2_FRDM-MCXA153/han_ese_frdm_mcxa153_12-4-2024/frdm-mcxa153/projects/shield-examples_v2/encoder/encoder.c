/*! ***************************************************************************
 *
 * \brief     Low level driver for the encoder
 * \file      encoder.c
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
#include "encoder.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile int32_t pulse_cnt = 0;
static volatile bool sw_pressed = false;
static volatile bool sw_released = false;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/*!
 * \brief Initializes the encoder pins
 *
 * Resources:
 * - ENC_A  | P3_31 | GPIO input with interrupts enabled on rising edges
 * - ENC_B  | P2_7  | GPIO input
 * - ENC_SW | P1_6  | GPIO input with interrupts enabled on both edges
 */
void encoder_init(void)
{
    // Enable modules and leave others unchanged
    // PORT1: [1] = Peripheral clock is enabled
    // GPIO1: [1] = Peripheral clock is enabled
    // PORT2: [1] = Peripheral clock is enabled
    // GPIO2: [1] = Peripheral clock is enabled
    // PORT3: [1] = Peripheral clock is enabled
    // GPIO3: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);

    // Release modules from reset and leave others unchanged
    // PORT1: [1] = Peripheral is released from reset
    // GPIO1: [1] = Peripheral is released from reset
    // PORT2: [1] = Peripheral is released from reset
    // GPIO2: [1] = Peripheral is released from reset
    // PORT3: [1] = Peripheral is released from reset
    // GPIO3: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    // Configure pins P1_6
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
    PORT1->PCR[6] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);

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
    GPIO1->ICR[6] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1011);

    // Enable GPIO1 interrupts
    NVIC_SetPriority(GPIO1_IRQn, 7);
    NVIC_ClearPendingIRQ(GPIO1_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);


    // Configure pins P2_7 and P3_31
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
    PORT2->PCR[7]  = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);
    PORT3->PCR[31] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);

    // 1. n.a.
    // 2. Initialize the interrupt function by writing to Interrupt Control a
    //    (ICR0 - ICR31) for the corresponding pins and desired configuration.
    //    If the pin is previously used for a different function, first write
    //    0100_0000h to Interrupt Control a (ICR0 - ICR31) to disable the
    //    previous function and clear the flag.

    // 2.
    //
    // ISF: [1] = Clear the flag
    // IRQC : [1010] = ISF and interrupt on rising edges
    GPIO3->ICR[31] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1001);

    // Enable GPIO3 interrupts
    NVIC_SetPriority(GPIO3_IRQn, 3);
    NVIC_ClearPendingIRQ(GPIO3_IRQn);
    NVIC_EnableIRQ(GPIO3_IRQn);
}

void GPIO1_IRQHandler(void)
{
    // Clear the interrupt
    NVIC_ClearPendingIRQ(GPIO1_IRQn);

    // Interrupt handler triggered by ENC_SW?
    if((GPIO1->ISFR[0] & GPIO_ISFR_ISF6(1)) != 0)
    {
        // Clear the flag
        GPIO1->ISFR[0] = GPIO_ISFR_ISF6(1);

        // Falling edge?
        if((GPIO1->PDIR & (1<<6)) == 0)
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

    // Interrupt handler triggered by ENC_A?
    if((GPIO3->ISFR[0] & GPIO_ISFR_ISF31(1)) != 0)
    {
        // Clear the flag
        GPIO3->ISFR[0] = GPIO_ISFR_ISF31(1);

        // Sample ENC_B
		if((GPIO2->PDIR & (1<<7)) == 0)
		{
			pulse_cnt--;
		}
		else
		{
			pulse_cnt++;
		}
    }
}

/*!
 * \brief Resets the counted pulses.
 *
 * Resets the internal pulses counter to 0.
 */
void encoder_reset(void)
{
	pulse_cnt = 0;
}

/*!
 * \brief Returns the number of pulses in CW or CCW direction since last reset
 *
 * The function keeps track of the CW and CCW pulses. For every CW pulse, an
 * internal counter is incremented. For every CCW pulse, that same counter is
 * decremented.
 *
 * Meaning:
 *   - If the function returns 0, no pulses were counted or as much CW as CCW
 *     pulses
 *   - If the function returns a value < 0, that much more number of CCW pulse
 *     were detected.
 *   - If the function returns a value > 0, that much more number of CW pulse
 *     were detected.
 *
 * \return The number of pulses counted since last reset
 */
int32_t encoder_pulses(void)
{
    return pulse_cnt;
}

/*!
 * \brief Detects if the switch was pressed.
 *
 * This firmware driver remembers if the switch was pressed with an internal
 * flag. When this function is called, it resets the internal flag if it was
 * true.
 *
 * \return True if the switch was pressed.
 */
bool encoder_sw_pressed(void)
{
    if(sw_pressed == true)
    {
        sw_pressed = false;
        return true;
    }
    
    return false;    
}

/*!
 * \brief Detects if the switch was released.
 *
 * This firmware driver remembers if the switch was released with an internal
 * flag. When this function is called, it resets the internal flag if it was
 * true.
 *
 * \return True if the switch was released.
 */
bool encoder_sw_released(void)
{
    if(sw_released == true)
    {
        sw_released = false;
        return true;
    }
    
    return false;  
}

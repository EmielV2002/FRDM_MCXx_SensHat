/*! ***************************************************************************
 *
 * \brief     Low level driver for the wake timer
 * \file      waketimer0.c
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
 ******************************************************************************/
#include "waketimer0.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
volatile bool waketimer0_timeout_flag = false;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void waketimer0_init(void)
{
    // Enables the corresponding FRO16 kHz output clock to other modules
    VBAT0->FROCLKE |= VBAT_FROCLKE_CLKE(1);
    
    // Enables the 4-bit clock divider to divide down the 16 kHz input clock to
    // generate the 1 kHz clock source for the wake timer. OSC_DIV_ENA must be
    // set to 1 for the wake timer to run.
    WAKETIMER0->WAKE_TIMER_CTRL |= WAKETIMER_WAKE_TIMER_CTRL_OSC_DIV_ENA(1) | 
        WAKETIMER_WAKE_TIMER_CTRL_INTR_EN(1);

	// Enable Interrupts
	NVIC_SetPriority(WAKETIMER0_IRQn, 0);
    NVIC_ClearPendingIRQ(WAKETIMER0_IRQn);
    NVIC_EnableIRQ(WAKETIMER0_IRQn);
}

void waketimer0_start(uint32_t val)
{
    // A write to this field pre-loads a start-count value into the timer and
    // starts a countdown sequence.
    WAKETIMER0->WAKE_TIMER_CNT = val;
}

void WAKETIMER0_IRQHandler(void)
{
	// Clear pending IRQ
	NVIC_ClearPendingIRQ(WAKETIMER0_IRQn);

    // Clear status flag by writing 1
    WAKETIMER0->WAKE_TIMER_CTRL |= WAKETIMER_WAKE_TIMER_CTRL_WAKE_FLAG_MASK;

    // Handle the event
    waketimer0_timeout_flag = true;
}

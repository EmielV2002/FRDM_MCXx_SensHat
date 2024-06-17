/*! ***************************************************************************
 *
 * \brief     Functions for generating task run-time statistics
 * \file      runtime_stats.h
 * \author    Hugo Arends
 * \date      March 2024
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
#include "runtime_stats.h"

volatile unsigned long ulHighFrequencyTicks = 0;

void vConfigureTimerForRunTimeStats( void )
{
    // Set clock source
	// MUX: [101] = CLK_1M 
    MRCC0->MRCC_LPTMR0_CLKSEL = MRCC_MRCC_LPTMR0_CLKSEL_MUX(0b101);
    
    // Set clock divider
    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_LPTMR0_CLKDIV = 0;
   
    // From section 34.5 Initialization (NXP, 2024)
    //
    // Perform the following procedure to initialize LPTMR:
    // 1. Configure Control Status (CSR) for the selected mode and pin
    //    configuration, when CSR[TEN] is 0. This resets the counter and clears
    //    the flag.
    // 2. Configure Prescaler and Glitch Filter (PSR) with the selected clock
    //    source and prescaler or glitch filter configuration.
    // 3. Configure Compare (CMR) with the selected compare point.
    // 4. Write 1 to CSR[TEN] to enable LPTMR.

    // 1.
    //
    // - TDRE : [0] = Timer DMA request disable
    // - TCF : [1] = Clears the Timer Compare Flag
    // - TIE : [0] = Timer interrupt disable
    // - TPS : [00] = Timer Pin Select is not used, leave at default value
    // - TPP : [0] = Timer Pin Polarity is not used, leave at default value
    // - TFC : [0] = CNR is reset whenever TCF is set
    // - TMS : [0] = Time Counter mode
    // - TEN : [0] = LPTMR is disabled
    LPTMR0->CSR = LPTMR_CSR_TCF(1);

    // 2.
    //
    // - PRESCALE : [0000] = n.a.
    // - PBYP : [1] = Prescaler and glitch filter disable
    // - PCS : [11] = Clock 3 is Combination of clocks configured in
    //                MRCC_LPTMR0_CLKSEL[MUX] field in SYSCON module. The Clock
    //                frequency must be less than 25 MHz to be used as a clock
    //                for the Low Power Timers. See Figure 60 
    LPTMR0->PSR = LPTMR_PSR_PBYP(1) | LPTMR_PSR_PCS(0b11);

    // 3.
    //
    // Generate an interrupt every millisecond
    LPTMR0->CMR = 1000-1;
   
    // 4.
    //
    // - TDRE : [0] = Timer DMA request disable
    // - TCF : [1] = Clears the Timer Compare Flag
    // - TIE : [1] = Timer interrupt enable
    // - TPS : [00] = Timer Pin Select is not used, leave at default value
    // - TPP : [0] = Timer Pin Polarity is not used, leave at default value
    // - TFC : [0] = CNR is reset whenever TCF is set
    // - TMS : [0] = Time Counter mode
    // - TEN : [1] = LPTMR is enable
    LPTMR0->CSR = LPTMR_CSR_TCF(1) | LPTMR_CSR_TIE(1) | LPTMR_CSR_TEN(1);

	// Enable Interrupts
	NVIC_SetPriority(LPTMR0_IRQn, 0);
    NVIC_ClearPendingIRQ(LPTMR0_IRQn);
    NVIC_EnableIRQ(LPTMR0_IRQn);
}

void LPTMR0_IRQHandler(void)
{
	// Clear pending IRQ
	NVIC_ClearPendingIRQ(LPTMR0_IRQn);

    // Clear status flag by writing 1
    LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;

    // Handle the event
    ulHighFrequencyTicks++;
}

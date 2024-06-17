/*! ***************************************************************************
 *
 * \brief     Buzzer driver
 * \file      buzzer.c
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
#include "buzzer.h"

// Note. P3_15 is connected to the buzzer, but there is no timer output function
//       connected to this pin. Instead, a timer is used to generated interrupts
//       and toggle the pin.

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
// Local function implementation
// -----------------------------------------------------------------------------
void buzzer_init(void)
{
    // Initialize LPTMR0
    // CLK_1M
    MRCC0->MRCC_LPTMR0_CLKSEL = MRCC_MRCC_LPTMR0_CLKSEL_MUX(0b101);
    MRCC0->MRCC_LPTMR0_CLKDIV = 0;
    LPTMR0->CSR = LPTMR_CSR_TCF(1);
    LPTMR0->PSR = LPTMR_PSR_PBYP(1) | LPTMR_PSR_PCS(0b11);
    LPTMR0->CMR = 0;
    LPTMR0->CSR = LPTMR_CSR_TCF(1) | LPTMR_CSR_TIE(1);

	// Enable Interrupts
	NVIC_SetPriority(LPTMR0_IRQn, 0);
    NVIC_ClearPendingIRQ(LPTMR0_IRQn);
    NVIC_EnableIRQ(LPTMR0_IRQn);

    // Initialize P3_15 (BUZZER pin)
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);

    PORT3->PCR[15] = PORT_PCR_LK(1);

    GPIO3->PCOR = (1<<15);
    GPIO3->PDDR |= (1<<15);
}

void LPTMR0_IRQHandler(void)
{
	// Clear pending IRQ
	NVIC_ClearPendingIRQ(LPTMR0_IRQn);

    // Clear status flag by writing 1
    LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;

    // Handle the event
    GPIO3->PTOR = (1<<15);
}

void buzzer_on(const uint16_t freq_hz)
{
    LPTMR0->CMR = 1000000 / (freq_hz * 2);
    LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;
}

void buzzer_off(void)
{
    LPTMR0->CSR &= ~LPTMR_CSR_TEN_MASK;
    GPIO3->PCOR = (1<<15);
}

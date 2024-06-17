/*! ***************************************************************************
 *
 * \brief     Low-Power Comparator (CMP) - interrupt
 * \file      cmp0_interrupt.c
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
#include "cmp0_interrupt.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile bool cmp0_trigger = false;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void cmp0_init(void)
{
    // System Power Control (SPC)
    //
    // Enable CMP0 and CMP0_DAC. See Figure 75 (NXP, 2024) for the relevant bit
    // positions.
    SPC0->ACTIVE_CFG1 = ((1 << 16) | (1 << 20));

    // Set clock source
	// MUX: [000] = FRO_12M
	MRCC0->MRCC_CMP0_RR_CLKSEL = MRCC_MRCC_CMP0_RR_CLKSEL_MUX(0);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_CMP0_FUNC_CLKDIV = 0;
    
    // Enable modules and leave others unchanged
    // CMP0: [1] = Peripheral clock is enabled
	// PORT2: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_CMP0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);

    // Release modules from reset and leave others unchanged
    // PORT2: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);

    // Configure P2_2
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [0] = Input buffer enable
    // MUX: [0000] = Alternative 0
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT2->PCR[2] = PORT_PCR_LK(1); // P2_2/ADC0_A4/CMP0_IN0

    // From section 41.5 Initialization (NXP, 2024)
    //
    // You can enable LPCMP by writing 1 to CCR0[CMP_EN], and then configuring
    // the control registers (CCR1, CCR2, DCR, and so on).

    //
    CMP0->CCR0 = LPCMP_CCR0_CMP_EN(1);

    // Select Continuous mode #2B - see Table 256 (NXP, 2024)
    //
    // FILT_PER: [00000000] = Bypass the filter
    // FILT_CNT: [000] = Filter is bypassed: COUT = COUTA
    // FUNC_CLK_SEL: [11] = CMPn function clock
    // EVT_SEL: [00] =  Rising edge
    // WINDOW_CLS: [0] = COUT event cannot close the window
    // WINDOW_INV: [0] = Do not invert
    // COUTA_OW: [0] = COUTA is 0
    // COUTA_OWEN: [0] = COUTA holds the last sampled value.
    // COUT_PEN: [0] = Comparator Output Pin disable
    // COUT_SEL: [0] = Use COUT (filtered)
    // COUT_INV: [0] = Do not invert
    // DMA_EN: [0] = Disable DMA
    // SAMPLE_EN: [0] = Disable Sampling mode
    // WINDOW_EN: [0] = Disable Windowing mode
    CMP0->CCR1 = LPCMP_CCR1_FUNC_CLK_SEL(0b11);

    // MSEL: [111] = Internal DAC output
    // PSEL: [000] = Input 0p
    // HYSTCTR: [01] = Level 1: Analog comparator hysteresis 10 mV.
    // CMP_NPMD: [0] = Disables CMP Nano power mode
    // CMP_HPMD: [0] = Low power (speed) comparison mode
    CMP0->CCR2 = LPCMP_CCR2_MSEL(0b111) | LPCMP_CCR2_HYSTCTR(0b01);

    // DAC_DATA: [01111111] = Selects the DAC output (DACO) voltage from one of
    //                        256 distinct levels by configuring the value of
    //                        DAC_DATA. The DACO ranges from Vin/256 to Vin
    //                        DACO = (Vin/256) * (DAC_DATA + 1)
    // VRSEL: [0] = VREFH0 (=VDD)
    // DAC_HPMD: [0] = Disable the DAC high power mode.
    // DAC_EN: [1] = Enable DAC
    CMP0->DCR = LPCMP_DCR_DAC_DATA(127) | LPCMP_DCR_DAC_EN(1);

    // RRF_IE: [0] = Disables the round-robin flag interrupt.
    // CFF_IE: [0] = Disables the comparator flag falling interrupt.
    // CFR_IE: [1] = Enables the comparator flag rising interrupt when CFR is
    //               set.
    CMP0->IER = LPCMP_IER_CFR_IE(1);

	// Enable Interrupts
	NVIC_SetPriority(CMP0_IRQn, 7);
    NVIC_ClearPendingIRQ(CMP0_IRQn);
    NVIC_EnableIRQ(CMP0_IRQn);
}

bool cmp0_triggered(void)
{
    // Copy local flag
    bool ret = cmp0_trigger;

    // Reset local flag
    cmp0_trigger = false;

    // Return copy
    return ret;
}

void CMP0_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(CMP0_IRQn);

    // Interrupt generated by a rising edge on COUT?
    if((CMP0->CSR & LPCMP_CSR_CFR_MASK) != 0)
    {
        // Clear the flag
        CMP0->CSR |= LPCMP_CSR_CFR_MASK;

        cmp0_trigger = true;
    }
}

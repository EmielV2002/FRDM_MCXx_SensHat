/*! ***************************************************************************
 *
 * \brief     Neopixel WS2812B driver - FlexPWM
 * \file      neopixel_flexpwm.c
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
#include "neopixel_flexpwm.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
// The total number of bits
#define N_BITS ((N_NEOPIXELS * sizeof(rgb_t) * 8) + 8)

// This driver assumes the main_clk is running at 96MHz. This driver uses
// interrupts and processing them must be finished before the next compare.
// value. Debugging showed that this is not possible with the main_clk running
// at 48 MHz
//
// The main_clk is divided by 2, so the FlexPWM module 2 is running at 48 MHz.
//
// The FlexPWM0 counter period is set to 60, which makes a PWM period of
// 1/48 MHz * (59+1) = 1.25us
//
// The following two defines set the output compare values for T0H and T1H:
#define T0H (15) // 1/48MHz * (15+1) = 330ns
#define T1H (30) // 1/48MHz * (30+1) = 645ns

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint16_t buffer[N_BITS];
static volatile uint32_t cnt = 0;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void neopixel_init(void)
{
    // Enable modules and leave others unchanged
    // FLEXPWM0: [1] = Peripheral clock is enabled
	// PORT3: [3] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_FLEXPWM0(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);

    // Release modules from reset and leave others unchanged
    // FLEXPWM0: [1] = Peripheral is released from reset
    // PORT3: [3] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_FLEXPWM0(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);

    // Enable FLEXPWM0 submodule clocks
    // CLKn_EN: [1] = Enable
    SYSCON->PWM0SUBCTL |= SYSCON_PWM0SUBCTL_CLK1_EN(1);

    // Configure P3_8
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [0] = Input buffer disable
    // MUX: [0101] = Alternative 5
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT3->PCR[8] = PORT_PCR_LK(1) | PORT_PCR_MUX(5); // PWM0_A1

    // Clear the Load Okay bit for submodule 1
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b010);

    // Configure submodule 1
    //
    // PRSC: [001] = Prescaler 2. PWM clock frequency = main_clk / 2 = 48 MHz
    // FULL: [1] = Full-cycle reloads enabled
    // Rest: default
    FLEXPWM0->SM[1].CTRL = PWM_CTRL_PRSC(0b001) | PWM_CTRL_FULL(1);

    // DBGEN: [1] = When set to one, the PWM continues to run while the chip is
    //              in Debug mode. If the device enters Debug mode and this bit
    //              is zero, then the PWM outputs are disabled until Debug mode
    //              is exited. At that point, the PWM pins resume operation as
    //              programmed in the PWM registers
    // INDEP: [1] = PWM_A and PWM_B outputs are independent PWMs
    // Rest: default
    FLEXPWM0->SM[1].CTRL2 = PWM_CTRL2_INDEP(1) | PWM_CTRL2_DBGEN(1);

    // POLX: [1] = PWM_X output inverted.
    // Rest: default
//    FLEXPWM0->SM[1].OCTRL = PWM_OCTRL_POL(1);

    // RIE: [1] = Reload Interrupt Enable
    // Rest: default
    FLEXPWM0->SM[1].INTEN = PWM_INTEN_RIE(1);

    // DISA: [0000] = Disable fault handling
    FLEXPWM0->SM[1].DISMAP[0] &= ~(PWM_DISMAP_DIS0A_MASK);

    // Edge aligned PWM. See reference manual paragraph 31.3.2.2 (NXP, 2024).
    FLEXPWM0->SM[1].INIT = 0;
    FLEXPWM0->SM[1].VAL0 = 0;
    FLEXPWM0->SM[1].VAL1 = 59;
    FLEXPWM0->SM[1].VAL2 = 0;
    FLEXPWM0->SM[1].VAL3 = 0;
    FLEXPWM0->SM[1].VAL4 = 0;
    FLEXPWM0->SM[1].VAL5 = 0;

    // Load prescaler, modulus, and PWM values of the submodule 1
	FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b010);

    // PWMX_EN: [010] = Enable PWM_X for submodule 1
    FLEXPWM0->OUTEN |= PWM_OUTEN_PWMA_EN(0b010);

	// Enable FlexPWM submodule 1 interrupts
	NVIC_SetPriority(FLEXPWM0_SUBMODULE1_IRQn, 0);
    NVIC_ClearPendingIRQ(FLEXPWM0_SUBMODULE1_IRQn);
    NVIC_EnableIRQ(FLEXPWM0_SUBMODULE1_IRQn);
}

void FLEXPWM0_SUBMODULE1_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(FLEXPWM0_SUBMODULE1_IRQn);

    // Clear the Load Okay bit for submodule 1
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b010);

    // Reload Flag clear
    FLEXPWM0->SM[1].STS = PWM_STS_RF(1);

    // Next compare value from the buffer
    FLEXPWM0->SM[1].VAL3 = buffer[++cnt];

    // Load prescaler, modulus, and PWM values of the submodule 1
	FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b010);

    // All data done?
    if(cnt == (N_BITS-1))
    {
        // PWM counter stopped
        FLEXPWM0->MCTRL = 0;
    }
}

void neopixel_update(const rgb_t *data)
{
    cnt = 0;
    
    // Extra bit(s) that will be low at the start
    buffer[cnt++] = 0;
    buffer[cnt++] = 0;
    buffer[cnt++] = 0;
    buffer[cnt++] = 0;

    // Prepare data: set output compare value for each bit
    for(uint32_t i=0; i < 12; ++i)
    {
        // Green
        // Loop all 8 bits, MSB first
        for(uint8_t j=0; j < 8; ++j)
        {
            // If the bit is clear, set T0H as output compare, otherwise T1H.
            buffer[cnt++] = ((data[i].g & (0x80 >> j)) == 0) ? T0H : T1H;
        }

        // Red
        // Loop all 8 bits, MSB first
        for(uint8_t j=0; j < 8; ++j)
        {
            // If the bit is clear, set T0H as output compare, otherwise T1H.
            buffer[cnt++] = ((data[i].r & (0x80 >> j)) == 0) ? T0H : T1H;
        }

        // Blue
        // Loop all 8 bits, MSB first
        for(uint8_t j=0; j < 8; ++j)
        {
            // If the bit is clear, set T0H as output compare, otherwise T1H.
            buffer[cnt++] = ((data[i].b & (0x80 >> j)) == 0) ? T0H : T1H;
        }
    }

    // Extra bit that will be low at the end
    buffer[cnt++] = 0;
    buffer[cnt++] = 0;
    buffer[cnt++] = 0;
    buffer[cnt++] = 0;
    
    cnt = 0;

    // Load initial compare value
	FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b010);
    FLEXPWM0->SM[1].VAL3 = buffer[cnt];
	FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b010);

    // PWM counter is started in the submodule 1
	FLEXPWM0->MCTRL |= PWM_MCTRL_RUN(0b010);
}

/*! ***************************************************************************
 *
 * \brief     Low level driver for the encoder
 * \file      servo.c
 * \author    Hugo Arends
 * \date      June 2024
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
#include "servo.h"

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

/*!
 * \brief Initializes the servo pins
 *
 * Resources:
 * - SERVO_PWM | P3_10 | CT1_MAT0
 *
 * Configures CTimer1 to generate a PWM signal on MAT0 with a 50 Hz frequency.
 */
void servo_init(void)
{
    // From section 26.1.5 Initialization (NXP, 2024)
    //
    // 1. Select a clock source for the CTIMER using MRCC_CTIMER0_CLKSEL,
    //    MRCC_CTIMER1_CLKSEL, and MRCC_CTIMER2_CLKSEL registers.
    // 2. Enable the clock to the CTIMER via the
    //    CTIMERGLOBALSTARTEN[CTIMER0_CLK_EN],
    //    CTIMERGLOBALSTARTEN[CTIMER1_CLK_EN], and
    //    CTIMERGLOBALSTARTEN[CTIMER2_CLK_EN] fields. This enables the register
    //    interface and the peripheral function clock.
    // 3. Clear the CTIMER peripheral reset using the MRCC_GLB_RST0 registers.
    // 4. Each CTIMER provides interrupts to the NVIC. See MCR and CCR registers
    //    in the CTIMER register section for match and capture events. For
    //    interrupt connections, see the attached NVIC spreadsheet.
    // 5. Select timer pins and pin modes as needed through the relevant PORT
    //    registers.
    // 6. The CTIMER DMA request lines are connected to the DMA trigger inputs
    //    via the DMAC0_ITRIG_INMUX registers (See Memory map and register
    //    definition). Note that timer DMA request outputs are connected to DMA
    //    trigger inputs.

    // 1.
    //
	// MUX: [101] = CLK_1M
	MRCC0->MRCC_CTIMER1_CLKSEL = MRCC_MRCC_CTIMER1_CLKSEL_MUX(0b101);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_CTIMER1_CLKDIV = 0;

    // 2.
    //
    // CTIMER1_CLK_EN: [1] = CTIMER 1 function clock enabled
    SYSCON->CTIMERGLOBALSTARTEN |= SYSCON_CTIMERGLOBALSTARTEN_CTIMER1_CLK_EN(1);

    // 3.
    //
    // Enable modules and leave others unchanged
    // CTIMER1: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_CTIMER1(1);

    // Release modules from reset and leave others unchanged
    // CTIMER1: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_CTIMER1(1);

    // 4.
    //
    // Not used

    // 5.
    //
    // Specifies the prescale value. 1 MHz / 1 = 1 MHz
    CTIMER1->PR = 0;

    // Match value: 1 MHz / 20000 = 50 Hz
    //
    // In PWM mode, use match channel 3 to set the PWM cycle length. The other
    // channels can be used for matches
    CTIMER1->MR[0] = 20000 - 1500;
    CTIMER1->MR[3] = 20000-1;

    // MR0S: [0] = Does not stop Timer Counter (TC) if MR0 matches Timer Counter
    //             (TC)
    // MR0R: [0] = Resets Timer Counter (TC) if MR0 matches its value.
    // MR0I: [0] = No interrupt when MR0 matches the value in Timer
    //             Counter (TC).
    // MR3S: [0] = Does not stop Timer Counter (TC) if MR3 matches Timer Counter
    //             (TC)
    // MR3R: [1] = Resets Timer Counter (TC) if MR3 matches its value.
    // MR3I: [0] = No interrupt when MR3 matches the value in Timer
    //             Counter (TC).
    CTIMER1->MCR |= CTIMER_MCR_MR3R(1);

    // Configure match outputs as PWM outputs.
    CTIMER1->PWMC |= CTIMER_PWMC_PWMEN3(1) | CTIMER_PWMC_PWMEN0(1);

    // Enable modules and leave others unchanged
    // PORT3: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);

    // Release modules from reset and leave others unchanged
    // PORT3: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);

    // Configure P3_10
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [0] = Disables
    // MUX: [0100] = Alternative 4 (CT1_MAT0)
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT3->PCR[10] = PORT_PCR_LK(1) | PORT_PCR_MUX(4);

    // 6.
    //
    // Not used.

    // CEN: [1] = Enables the counters.
    CTIMER1->TCR |= CTIMER_TCR_CEN(1);

    // Configuration summary
    //
    // CLK_1M = 1 MHz
    // ctimer1_prescaler = 1
    // ctimer1_count_frequency = 1 MHz / 1 = 1 MHz
    // ctimer1_modulo_value = ctimer1_match3 = 20000 - 1
    // ctimer1_pwm_frequency = ctimer1_count_frequency / (ctimer1_match3 + 1)
    //                       = 1 MHz / 20000
    //                       = 50 Hz
    // ctimer1_duty_cycle: from 0 to ctimer1_match3

    // CTIMER1
    //
    // MAT3 (20000-1) - - - - - - - - - - - - - -+- - - - - - - - - - - -+ -
    //                                          /|                      /|
    //                                         / |                     / |
    //                                        /  |                    /  |
    //                                       /   |                   /   |
    //                                      /    |                  /    |
    // MAT0 (20000-1500) - - - - - - - - - + - - | - - - - - - - - + - - | -
    //                                    /.     |                /.     |
    //                                   / .     |               / .     |
    //                                  /  .     |              /  .     |
    //                                 /   .     |             /   .     |
    //                                /    .     |            /    .     |
    //                               /     .     |           /     .     |
    //                              /      .     |          /      .     |
    //                             /       .     |         /       .     |
    //                            /        .     |        /        .     |
    //                           /         .     |       /         .     |
    //                          /          .     |      /          .     |
    //                         /           .     |     /           .     |
    //                        /            .     |    /            .     |
    //                       /             .     |   /             .     |
    //                      /              .     |  /              .     |
    //                     /               .     | /               .     |
    //                    /                .     |/                .     |/
    // 0 ----------------+-----------------------+-----------------------+--
    //                 __.                 ._____.                 ._____.
    // CT1_MAT0        //|_________________|/////|_________________|/////|__
    //                   .                 .     .                 .     .
    //                   |<--------------->|<--->|<--------------->|<--->|
    //                         18.5ms       1.5ms      18.5ms       1.5ms
}

/*!
 * \brief Set the servo duty cycle
 *
 * Sets the PWM duty cycle as follows:
 * - 1000 =   0% = 1.0 ms pulse width = servo moves left
 * - 1500 =  50% = 1.5 ms pulse width = servo moves centre
 * - 2000 = 100% = 2.0 ms pulse width = servo moves right
 *
 * \param[in]  value Duty cycle of the PWM signal
 */
void servo_set(int32_t value)
{
	// Check range
	value = value < 1000 ? 1000 : value;
	value = value > 2000 ? 2000 : value;

	// Set new match value
	CTIMER1->MR[0] = 20000 - value;
}

/*! ***************************************************************************
 *
 * \brief     FLEXPWM0 - Edge aligned
 * \file      flexpwm0_edge_alligned.c
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
 ******************************************************************************/
#include "flexpwm0_edge_aligned.h"

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
void flexpwm0_init(void)
{
    // FLEXPWM clock source is fixed: main_clk. See reference manual Figure 64
    //(NXP, 2024).

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
    SYSCON->PWM0SUBCTL |= SYSCON_PWM0SUBCTL_CLK0_EN(1);
    SYSCON->PWM0SUBCTL |= SYSCON_PWM0SUBCTL_CLK1_EN(1);

    // Configure P3_0, P3_12 and P3_13
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
    PORT3->PCR[0] = PORT_PCR_LK(1) | PORT_PCR_MUX(5); // PWM0_A0
    PORT3->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_MUX(5); // PWM0_X0
    PORT3->PCR[13] = PORT_PCR_LK(1) | PORT_PCR_MUX(5); // PWM0_X1

    // Clear the Load Okay bit for submodules 0 and 1
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b011);

    // Configure submodule 0
    //
    // PRSC: [001] = Prescaler 2. PWM clock frequency = main_clk / 2
    // FULL: [1] = Full-cycle reloads enabled
    // Rest: default
    FLEXPWM0->SM[0].CTRL = PWM_CTRL_PRSC(0b001) | PWM_CTRL_FULL(1);

    // DBGEN: [1] = When set to one, the PWM continues to run while the chip is
    //              in Debug mode. If the device enters Debug mode and this bit
    //              is zero, then the PWM outputs are disabled until Debug mode
    //              is exited. At that point, the PWM pins resume operation as
    //              programmed in the PWM registers
    // INDEP: [1] = PWM_A and PWM_B outputs are independent PWMs
    // Rest: default
    FLEXPWM0->SM[0].CTRL2 = PWM_CTRL2_INDEP(1) | PWM_CTRL2_DBGEN(1);

    // POLA: [1] = PWM_A output inverted. A low level on the PWM_A pin
    //             represents the "on" or "active" state
    // POLX: [0] = Default value. Do not invert polarity for PWM_X, because
    //             active state is controlled by VAL0
    // Rest: default
    FLEXPWM0->SM[0].OCTRL = PWM_OCTRL_POLA(1);

    // DISA: [0000] = Disable fault handling
    // DISX: [0000] = Disable fault handling
    FLEXPWM0->SM[0].DISMAP[0] &= ~(PWM_DISMAP_DIS0A_MASK);
    FLEXPWM0->SM[0].DISMAP[0] &= ~(PWM_DISMAP_DIS0X_MASK);

    // Edge aligned PWM. See reference manual paragraph 31.3.2.2 (NXP, 2024).
    FLEXPWM0->SM[0].INIT = 0;
    FLEXPWM0->SM[0].VAL0 = 0;
    FLEXPWM0->SM[0].VAL1 = 0xFFFF;
    FLEXPWM0->SM[0].VAL2 = 0;
    FLEXPWM0->SM[0].VAL3 = 0;
    FLEXPWM0->SM[0].VAL4 = 0;
    FLEXPWM0->SM[0].VAL5 = 0;

    // Configure submodule 1
    //
    // PRSC: [001] = Prescaler 2. PWM clock frequency = main_clk / 2
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

    // POLX: [0] = Default value. Do not invert polarity for PWM_X, because
    //             active state is controlled by VAL0
    // Rest: default

    // DISX: [0000] = Disable fault handling
    FLEXPWM0->SM[1].DISMAP[0] &= ~(PWM_DISMAP_DIS0X_MASK);

    // Edge aligned PWM. See reference manual paragraph 31.3.2.2 (NXP, 2024).
    FLEXPWM0->SM[1].INIT = 0;
    FLEXPWM0->SM[1].VAL0 = 0;
    FLEXPWM0->SM[1].VAL1 = 0xFFFF;
    FLEXPWM0->SM[1].VAL2 = 0;
    FLEXPWM0->SM[1].VAL3 = 0;
    FLEXPWM0->SM[1].VAL4 = 0;
    FLEXPWM0->SM[1].VAL5 = 0;
    
    // Load prescaler, modulus, and PWM values of the submodules 0 and 1
	FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b011);

    // PWMA_EN: [001] = Enable PWM_A for submodule 0
    // PWMX_EN: [011] = Enable PWM_X for submodule 0 and submodule 1
    FLEXPWM0->OUTEN |= PWM_OUTEN_PWMA_EN(0b001) | PWM_OUTEN_PWMX_EN(0b011);

    // PWM counter is started in the submodules 0 and 1
	FLEXPWM0->MCTRL |= PWM_MCTRL_RUN(0b011);


    // Configuration summary
    //
    // main_clk = 48 MHz
    // submodule_0_1_prescaler = 2
    // submodule_0_1_count_frequency = 48 MHz / 2 = 24 MHz
    // submodule_0_1_modulo_value = VAL1 = 0xFFFF
    // submodule_0_1_pwm_frequency = submodule_0_1_count_frequency / (VAL1 + 1)
    //                             = 24 MHz / 65536
    //                             = 366.21 Hz
    // submodule_0_1_duty_cycle: from 0 to VAL1

    // Submodule 0
    //
    // VAL1 (0xFFFF)  - - - - - - - - - - - - - -+- - - - - - - - - - - -+ -
    //                                          /|                      /|
    //                                         / |                     / |
    //                                        /  |                    /  |
    //                                       /   |                   /   |
    //                                      /    |                  /    |
    // VAL0  - - - - - - - - - - - - - - - + - - | - - - - - - - - + - - | -
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
    // VAL3  - - - - - - - - - +  - - - -  . - - | - - +  - - - -  . - - | -
    //                        /.           .     |    /.           .     |
    //                       / .           .     |   / .           .     |
    //                      /  .           .     |  /  .           .     |
    //                     /   .           .     | /   .           .     | /
    //                    /    .           .     |/    .           .     |/
    // INIT (0x0000) ----+-----------------------+-----------------------+--
    // VAL2 (0x0000)     .     .           .     .     .           .     .
    //                   .     .           .     .     .           .     .
    //                   .     .           .     .     .           .     .
    //                    _____            .      _____            .      __
    // PWM_A           __|     |_________________|     |_________________|
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    // (PWM_B)           .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                 __                   _____                   _____
    // PWM_X             |_________________|     |_________________|     |__


    // Submodule 1
    //
    // VAL1 (0xFFFF)  - - - - - - - - - - - - - -+- - - - - - - - - - - -+ -
    //                                          /|                      /|
    //                                         / |                     / |
    //                                        /  |                    /  |
    //                                       /   |                   /   |
    //                                      /    |                  /    |
    // VAL0  - - - - - - - - - - - - - - - + - - | - - - - - - - - + - - | -
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
    //                     /               .     | /               .     | /
    //                    /                .     |/                .     |/
    // INIT (0x0000) ----+-----------------------+-----------------------+--
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    // (PWM_A)           .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    // (PWM_B)           .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                   .                 .     .                 .     .
    //                 __                   _____                   _____
    // PWM_X             |_________________|     |_________________|     |__

}

inline void flexpwm0_set_red(const uint16_t duty_cycle)
{
    // LED_RED: P3_12/PWM0_X0

    // Clear the Load Okay bit for submodule 0
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b001);

    // Set duty cycle
    FLEXPWM0->SM[0].VAL0 = duty_cycle;

    // Load prescaler, modulus, and PWM values of the submodule 0
    FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b001);
}

inline void flexpwm0_set_green(const uint16_t duty_cycle)
{
    // LED_GREEN: P3_13/PWM0_X0
    
    // Clear the Load Okay bit for submodule 1
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b010);

    // Set duty cycle
    FLEXPWM0->SM[1].VAL0 = duty_cycle;

    // Load prescaler, modulus, and PWM values of the submodule 1
    FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b010);
}

inline void flexpwm0_set_blue(const uint16_t duty_cycle)
{
    // LED_BLUE: P3_0/PWM0_A0

    // Clear the Load Okay bit for submodule 0
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b001);

    // Set duty cycle
    FLEXPWM0->SM[0].VAL3 = duty_cycle;

    // Load prescaler, modulus, and PWM values of the submodule 0
    FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b001);
}

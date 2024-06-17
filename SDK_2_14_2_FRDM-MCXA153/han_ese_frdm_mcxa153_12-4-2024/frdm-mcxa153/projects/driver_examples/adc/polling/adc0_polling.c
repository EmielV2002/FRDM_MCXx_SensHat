/*! ***************************************************************************
 *
 * \brief     Analog-to-Digital Converter (ADC) - polling
 * \file      adc0_polling.c
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
#include "adc0_polling.h"

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
void adc0_init(void)
{
    // Set clock source
	// MUX: [000] = FRO_12M
	MRCC0->MRCC_ADC0_CLKSEL = MRCC_MRCC_ADC0_CLKSEL_MUX(0);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_ADC0_CLKDIV = 0;

    // Enable modules and leave others unchanged
    // ADC0: [1] = Peripheral clock is enabled
	// PORT1: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_ADC0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);

    // Release modules from reset and leave others unchanged
    // ADC0: [1] = Peripheral is released from reset
    // PORT1: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_ADC0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);

    // Configure P1_10
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
    PORT1->PCR[10] = PORT_PCR_LK(1); // P1_10/ADC0_A8

    // ADC Analog Pre-Enable
    // The PUDLY must be programmed to a non-zero value to enable the ADC.
    // Voltage Reference Selection: Option 3 setting
    ADC0->CFG = ADC_CFG_PWREN(1) | ADC_CFG_PUDLY(0x80) | ADC_CFG_REFSEL(0b10);

    // From section 40.5 Initialization (NXP, 2024)
    //
    // Calibration
    // 1. ADC must be enabled (CTRL[ADCEN] = 1h) before a calibration function
    //    runs.

    // 1.
    //
    ADC0->CTRL |= ADC_CTRL_ADCEN(1);

    // Offset calibration
    // 1. Configure for the desired averaging via CTRL[CAL_AVGS]. The minimum
    //    recommended setting is for 256 averaging (CTRL[CAL_AVGS] set to 0x8 or
    //    greater).
    // 2. Initiate Offset Calibration by setting CTRL[CALOFS].
    // 3. Poll the STAT[CAL_RDY] flag. When STAT[CAL_RDY] is asserted, the
    //    offset calibration function has completed and the OFSTRIM register has
    //    updated.

    // 1.
    //
    ADC0->CTRL |= ADC_CTRL_CAL_AVGS(0b1010);

    // 2.
    //
    ADC0->CTRL |= ADC_CTRL_CALOFS(1);

    // 3.
    //
    while((ADC0->STAT & ADC_STAT_CAL_RDY_MASK) == 0)
    {}

    // ADC Calibration
    // 1. Execute Offset calibration steps. The Offset Trim Register (OFSTRIM)
    //    is used during calibration to trim for comparator offset voltage.
    // 2. Configure for the desired averaging via CTRL[CAL_AVGS]. The minimum
    //    recommended setting is for 256 averaging (CTRL[CAL_AVGS] set to 0x8 or
    //    greater).
    // 3. Initiate the calibration routine by writing 1 to CTRL[CAL_REQ].
    //    CTRL[CAL_REQ] remains 1 until the CAL routine has been accepted by the
    //    ADC. After acceptance, CTRL[CAL_REQ] automatically becomes 0.
    // 4. Poll the GCR0[RDY] flag. When it is asserted, the hardware controlled
    //    A-side calibration operation is complete and CAL_GAR and
    //    GCC0[GAIN_CAL] registers are updated. The updated value in
    //    GCC0[GAIN_CAL] is needed for further software processing described in
    //    the following steps.
    // 5. Read GCC0[GAIN_CAL] and store for use in the gain_adjustment
    //    calculation.
    // 6. Calculate the
    //    A-side gain_adjustment = (131072)/(131072-GCC0[GAIN_CAL]).
    //    GCC0[GAIN_CAL] is a 16-bit signed value. This results in a floating
    //    point value between 0 and 2.
    // 7. Convert the floating point value to its integer component (0 or 1) and
    //    its fractional component rounded to 16-bits. The integer value is
    //    stored to GCR0[GCALR[16]] and the fractional component is stored to
    //    GCR0[GCALR[15:0]]. Write this value to the GCR0[GCALR] register.
    // 8. Once GCR0[GCALR] contains the result from the gain_adjustment
    //    calculation, set the GCR0[RDY] flag to indicate it is valid.

    // 1.
    //
    // Done, see above

    // 2.
    //
    ADC0->CTRL |= ADC_CTRL_CAL_AVGS(0b1010);

    // 3.
    //
    ADC0->CTRL |= ADC_CTRL_CAL_REQ(1);

    // 4.
    //
    // NOTE: The reference manual states: "Poll the GCR0[RDY] flag." This does
    //       not seem to work and the provided fsl driver in the SDK checks the
    //       GCC0[RDY] instead. For that reason, GCC0[RDY] is checked here.
    while((ADC0->GCC[0] & ADC_GCC_RDY_MASK) == 0)
    {}
    
    // 5.
    //
    uint16_t value = (uint16_t)ADC0->GCC[0];

    // 6.
    //
    float gain_adjustment = (131072.0f)/(131072.0f-(float)value);

    // 7.
    //
    uint32_t gain_calc_result_array[17] = {0};

    for(uint32_t i = 17; i > 0; i--)
    {
        uint32_t tmp = (uint32_t)((gain_adjustment) /
            ((1.0f / (float)(1U << (0x10U - (i - 1U))))));
        gain_calc_result_array[i - 1U] = tmp;
        gain_adjustment = gain_adjustment - ((float)tmp) *
            ((1.0f / (float)(1U << (0x10U - (i - 1U)))));
    }

    uint32_t gain_calc_result = 0;

    for(uint32_t i = 17; i > 0U; i--)
    {
        gain_calc_result += gain_calc_result_array[i - 1U] *
            ((uint32_t)(1UL << (uint32_t)(i - 1UL)));
    }

    ADC0->GCR[0] = ADC_GCR_GCALR(gain_calc_result);

    // 8.
    //
    ADC0->GCR[0] |= ADC_GCR_RDY(1);

    // Create CMD1:
    // - High resolution. Single-ended 16-bit conversion.
    // - Channel 8
    ADC0->CMD[0].CMDL = ADC_CMDL_MODE(1) | ADC_CMDL_ADCH(8);

    // Create trigger 0:
    // - Select CMD1
    // - Rest default
    ADC0->TCTRL[0] = ADC_TCTRL_TCMD(0b001);
}

uint16_t adc0_sample_channel_08(void)
{
    // Initiate software trigger 0 conversion
    ADC0->SWTRIG = ADC_SWTRIG_SWT0(1);

    // Wait for the conversion to complete. Store the value from ADC0->RESFIFO,
    // beacuse every successive read empties the FIFO.
    uint32_t result = 0;

    while((result & ADC_RESFIFO_VALID_MASK) == 0)
    {
        result = ADC0->RESFIFO;
    }

    // Return the result
    return (uint16_t)result;
}

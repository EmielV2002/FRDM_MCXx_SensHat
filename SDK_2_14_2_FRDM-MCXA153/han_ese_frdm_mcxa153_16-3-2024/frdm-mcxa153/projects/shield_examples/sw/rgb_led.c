/*! ***************************************************************************
 *
 * \brief     RGB led
 * \file      rgb_led.c
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
#include "rgb_led.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
// Lookup table for the RGB LEDs pins. All pins are on P2.
static const uint8_t pin_lut[3] = 
{
    0, // P2_0 - LED_R
    2, // P2_2 - LED_G
    1, // P2_1 - LED_B
};

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void rgb_led_init(void)
{
    // Initialize pins
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);

    PORT2->PCR[pin_lut[0]] = PORT_PCR_LK(1);
    PORT2->PCR[pin_lut[1]] = PORT_PCR_LK(1);
    PORT2->PCR[pin_lut[2]] = PORT_PCR_LK(1);

    GPIO2->PDOR |= (1<<pin_lut[0]) | (1<<pin_lut[1]) | (1<<pin_lut[2]);
    GPIO2->PDDR |= (1<<pin_lut[0]) | (1<<pin_lut[1]) | (1<<pin_lut[2]);
}

void rgb_led_on(const rgb_led_t led)
{
    GPIO2->PCOR = (1<<pin_lut[led]);
}

void rgb_led_off(const rgb_led_t led)
{
    GPIO2->PSOR = (1<<pin_lut[led]);
}

void rgb_led_toggle(const rgb_led_t led)
{
    GPIO2->PTOR = (1<<pin_lut[led]);
}

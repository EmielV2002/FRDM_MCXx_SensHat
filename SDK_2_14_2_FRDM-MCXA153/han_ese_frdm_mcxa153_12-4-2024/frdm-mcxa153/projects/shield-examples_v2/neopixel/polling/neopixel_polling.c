/*! ***************************************************************************
 *
 * \brief     Neopixel SK6812 driver - polling
 * \file      neopixel_poling.c
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
#include "neopixel_polling.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef NDEBUG

// Values are valid only when:
// - Core clock is 48 MHz
// - Optimization is set to -Ofast
#define T0H (20) // 0.3us
#define T1H (45) // 0.6us
#define T0L (70) // 0.9us
#define T1L (36) // 0.6us

#else

#error "Timing will not work as designed"

#endif

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void neopixel_init(void)
{
    // Initialize P3_8 (LED_NEOPIXEL pin)
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    PORT3->PCR[8] = PORT_PCR_LK(1);

    GPIO3->PDOR &= ~(1<<8);
    GPIO3->PDDR |= (1<<8);
}

void neopixel_update(const rgb_t *data)
{
    // Critical section
    uint32_t m = __get_PRIMASK();
    __disable_irq();

	// Loop all pixels
	for(uint32_t i=0; i < N_NEOPIXELS; ++i)
	{
		// Green
		for(uint8_t j=0; j < 8; ++j)
		{
            // Set pin
            GPIO3->PSOR = (1<<8);

            // Is this bit 0 or 1?
			if((data[i].g & (0x80 >> j)) == 0)
			{
                // 0 bit - wait T0H
                for(uint32_t k=0; k<T0H; k++)
                {
                    __asm("nop");
                }
			}
			else
			{
                // 1 bit - wait T1H
                for(uint32_t k=0; k<T1H; k++)
                {
                    __asm("nop");
                }
			}

            // Clear pin
            GPIO3->PCOR = (1<<8);

            // Is this bit 0 or 1?
			if((data[i].g & (0x80 >> j)) == 0)
			{
                // 0 bit - wait T0L
                for(uint32_t k=0; k<T0L; k++)
                {
                    __asm("nop");
                }
			}
			else
			{
                // 1 bit - wait T1L
                for(uint32_t k=0; k<T1L; k++)
                {
                    __asm("nop");
                }
			}
		}

		// Red
		for(uint8_t j=0; j < 8; ++j)
		{
            // Set pin
            GPIO3->PSOR = (1<<8);

            // Is this bit 0 or 1?
			if((data[i].r & (0x80 >> j)) == 0)
			{
                // 0 bit - wait T0H
                for(uint32_t k=0; k<T0H; k++)
                {
                    __asm("nop");
                }
			}
			else
			{
                // 1 bit - wait T1H
                for(uint32_t k=0; k<T1H; k++)
                {
                    __asm("nop");
                }
			}

            // Clear pin
            GPIO3->PCOR = (1<<8);

            // Is this bit 0 or 1?
			if((data[i].r & (0x80 >> j)) == 0)
			{
                // 0 bit - wait T0L
                for(uint32_t k=0; k<T0L; k++)
                {
                    __asm("nop");
                }
			}
			else
			{
                // 1 bit - wait T1L
                for(uint32_t k=0; k<T1L; k++)
                {
                    __asm("nop");
                }
			}
		}

		// Blue
		for(uint8_t j=0; j < 8; ++j)
		{
            // Set pin
            GPIO3->PSOR = (1<<8);

            // Is this bit 0 or 1?
			if((data[i].b & (0x80 >> j)) == 0)
			{
                // 0 bit - wait T0H
                for(uint32_t k=0; k<T0H; k++)
                {
                    __asm("nop");
                }
			}
			else
			{
                // 1 bit - wait T1H
                for(uint32_t k=0; k<T1H; k++)
                {
                    __asm("nop");
                }
			}

            // Clear pin
            GPIO3->PCOR = (1<<8);

            // Is this bit 0 or 1?
			if((data[i].b & (0x80 >> j)) == 0)
			{
                // 0 bit - wait T0L
                for(uint32_t k=0; k<T0L; k++)
                {
                    __asm("nop");
                }
			}
			else
			{
                // 1 bit - wait T1L
                for(uint32_t k=0; k<T1L; k++)
                {
                    __asm("nop");
                }
			}
		}
	}

	__set_PRIMASK(m);
}

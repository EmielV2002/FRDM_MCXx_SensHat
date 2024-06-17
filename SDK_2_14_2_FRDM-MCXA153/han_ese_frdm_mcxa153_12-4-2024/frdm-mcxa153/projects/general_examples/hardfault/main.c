/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
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
 *****************************************************************************/
#include <MCXA153.h>

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
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // Initialize P3_12 and P3_13 GPIO output
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    PORT3->PCR[12] = PORT_PCR_LK(1);
    PORT3->PCR[13] = PORT_PCR_LK(1);
    GPIO3->PSOR = (1<<12) | (1<<13);
    GPIO2->PDDR |= (1<<12) | (1<<13);
    
    // Enable interrupts
    __enable_irq();

    while(1)
    {
        
        // LED green on
        // 0x40105044 is the address of GPIO3->PCOR in the memory map taken 
        // from the reference manual
        *((uint32_t *)(0x40105044)) = (1UL<<13);

        // Delay
        for(volatile int i=0; i<1000000; i++)
        {}
        
        // LED green off
        // The next register is PSOR, see reference manual
        *((uint32_t *)(0x40105045)) = (1UL<<13);
        
        // Delay
        for(volatile int i=0; i<1000000; i++)
        {}        
    }    
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

// If this interrupt handler gets executed, a hardfault has occurred.
// We can check this by setting a breakpoint in this HardFault_Handler function 
// and start the debugger to see if code execution stops at the breakpoint.
void HardFault_Handler(void)
{
    while(1)
    {
        // Do nothing
    }
}

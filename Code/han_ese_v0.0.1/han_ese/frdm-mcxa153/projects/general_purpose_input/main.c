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
#include <stdbool.h>
#include "general_purpose_output.h"
#include "general_purpose_input.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
//#define FINAL_ASSIGNMENT

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
// Final assignment
#ifdef FINAL_ASSIGNMENT
static bool sw3_previous_clicked = false;
static bool sw3_current_clicked = false;
#endif

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    gpi_init();
    gpo_init();

    // TODO Assignment III
    #ifndef FINAL_ASSIGNMENT
    {
    while(1)
    {
        // SW3 pressed?
        if((GPIO1->PDIR & (1<<7)) == 0)
        {
            // Blue LED on
            GPIO3->PCOR = (1<<0);
        }
        else
        {
            // Blue LED off
            GPIO3->PSOR = (1<<0);
        }
    }
    }
    #endif
    
    // Final assignment
    #ifdef FINAL_ASSIGNMENT
    {
    while(1)
    {
        // Save current pin state
        sw3_current_clicked = (GPIO1->PDIR & (1<<7)) == 0;

        // Different pin state from last time checked?
        if(sw3_current_clicked != sw3_previous_clicked)
        {
            // Save this state
            sw3_previous_clicked = sw3_current_clicked;

            // SW3 clicked
            if(sw3_current_clicked == true)
            {
                // Blue LED toggle
                GPIO3->PTOR = (1<<0);
            }
        }
    }
    }
    #endif
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
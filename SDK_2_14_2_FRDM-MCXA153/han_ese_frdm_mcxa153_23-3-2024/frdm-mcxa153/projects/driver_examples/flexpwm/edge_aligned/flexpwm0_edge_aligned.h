/*! ***************************************************************************
 *
 * \brief     FLEXPWM0 - Edge aligned
 * \file      flexpwm0_edge_alligned.h
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
#ifndef FLEXPWM0_H
#define FLEXPWM0_H

#include <MCXA153.h>

// -----------------------------------------------------------------------------
// Shared type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Shared variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Shared function prototypes
// -----------------------------------------------------------------------------
void flexpwm0_init(void);

void flexpwm0_set_red(const uint16_t duty_cycle);
void flexpwm0_set_green(const uint16_t duty_cycle);
void flexpwm0_set_blue(const uint16_t duty_cycle);

#endif // FLEXPWM0_H

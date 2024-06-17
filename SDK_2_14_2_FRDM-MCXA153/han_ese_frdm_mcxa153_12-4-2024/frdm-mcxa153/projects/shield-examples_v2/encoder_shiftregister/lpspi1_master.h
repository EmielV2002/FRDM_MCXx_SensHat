/*! ***************************************************************************
 *
 * \brief     Low level driver for the Low Power Serial Peripheral Interface
 *            (LPSPI) in master mode
 * \file      lpspi1_master.h
 * \author    Hugo Arends
 * \date      May 2024
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
#ifndef LPSPI1_MASTER_H
#define LPSPI1_MASTER_H

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
void lpspi1_master_init(void);
void lpspi1_master_tx(uint8_t *buffer, const uint32_t n);
void lpspi1_master_rx_read(uint8_t *buffer, const uint32_t n);

#endif // LPSPI1_MASTER_H

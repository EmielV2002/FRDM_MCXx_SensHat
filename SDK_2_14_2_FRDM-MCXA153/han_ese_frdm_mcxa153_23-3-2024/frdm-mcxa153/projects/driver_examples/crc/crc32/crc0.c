/*! ***************************************************************************
 *
 * \brief     Cyclic Redundancy Check (CRC)
 * \file      crc0.c
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
#include "crc0.h"

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
void crc0_init(void)
{
    // Enable modules and leave others unchanged
    // CRC: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_CRC(1);

    // Release modules from reset and leave others unchanged
    // CRC: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_CRC(1);
    
    // From section 44.6 Use cases (NXP, 2024) Table 273. CTRL programming for
    // 32-bit CRC
    //
    // Algorithm | Polynomial | Seed      | Ref in | Ref out | XOR out
    // CRC-32    | 04C11DB7h  | FFFFFFFFh | 1      | 1       | FFFFFFFFh
    //
    // CTRL[TOT] | CTRL[TOTR] | CTRL[FXOR]
    // 1h        | 2h         | 1h

    // From section 44.5 Initialization (NXP, 2024)
    //
    // To enable CRC calculation, you must program:
    // 1. CTRL[WAS] 
    // 2. Polynomial (GPOLY).
    // 3. Parameters for transposition and CRC result inversion in the
    //    applicable registers.
    
    // 1. and 3.
    //
    // TOT : [01] = Bits in bytes are transposed, but bytes are not transposed.
    // TOTR : [10] = Both bits in bytes and bytes are transposed.
    // FXOR : [1] = Inverts or complements the read value of the CRC Data.
    // WAS : [0] = Data values
    // TCRC : [1] = 32-bits
    CRC0->CTRL = CRC_CTRL_TCRC(1) | CRC_CTRL_TOT(1) | CRC_CTRL_TOTR(2) |
        CRC_CTRL_FXOR(1);

    // 2.
    //
    // Write polynomial
    CRC0->GPOLY = 0x04C11DB7UL;
}

uint32_t crc0_calculate(const uint32_t *data, const uint32_t n)
{
    // From section 44.5 Initialization (NXP, 2024)
    //
    // Writing 1 to CTRL[WAS] enables you to program the seed value into CRC
    // Data registers.
    // After writing all the data, you must wait for at least two clock cycles
    // to read the data from CRC Data (DATA) register.
    // After a CRC calculation completes, you can reinitialize the module for a
    // new CRC computation by again writing 1 to CTRL[WAS] and programming a 
    // new, or previously used, seed value. You must set all other parameters
    // before programming the seed value and subsequent data values.

    // WAS : [1] = Seed value(s)
    CRC0->CTRL |= CRC_CTRL_WAS_MASK;
    
    // Write seed
    CRC0->DATA = 0xFFFFFFFFUL;
    
    // WAS : [0] = Data value(s)
    CRC0->CTRL &= ~CRC_CTRL_WAS_MASK;
    
    for(uint32_t i=0; i<n; i++)
    {
        CRC0->DATA = data[i];
    }
    
    return CRC0->DATA;
}
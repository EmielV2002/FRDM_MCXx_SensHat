/*! ***************************************************************************
 *
 * \brief     Low level driver for the FXLS8974CF
 * \file      fxls8974cf.h
 * \author    Hugo Arends
 * \date      April 2024
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
#ifndef FXLS8974CF_H
#define FXLS8974CF_H

#include <MCXA153.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Shared type definitions
// -----------------------------------------------------------------------------
#define FXLS8974CF_INT_STATUS   (0x00)
#define FXLS8974CF_TEMP_OUT     (0x01)
#define FXLS8974CF_WHO_AM_I     (0x13)
#define FXLS8974CF_SENS_CONFIG1 (0x15)
#define FXLS8974CF_SENS_CONFIG2 (0x16)
#define FXLS8974CF_SENS_CONFIG3 (0x17)
#define FXLS8974CF_SENS_CONFIG4 (0x18)
#define FXLS8974CF_SENS_CONFIG5 (0x19)
#define FXLS8974CF_SENS_INT_EN  (0x20)
#define FXLS8974CF_SENS_PIN_SEL (0x21)

// -----------------------------------------------------------------------------
// Shared variables
// -----------------------------------------------------------------------------
extern volatile bool fxls8974cf_int0;

// -----------------------------------------------------------------------------
// Shared function prototypes
// -----------------------------------------------------------------------------
void fxls8974cf_init(void);

void fxls8974cf_read(uint8_t reg, uint8_t *data, uint8_t len);
void fxls8974cf_write(uint8_t reg, uint8_t *data, uint8_t len);

#endif // FXLS8974CF_H

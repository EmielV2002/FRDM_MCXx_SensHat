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
#include "accelerometer.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#define N_DATA (4+2)

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static uint8_t tx_data[N_DATA];
static uint8_t rx_data[N_DATA];

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    accelerometer_init();
    
    // -------------------------------------------------------------------------
    // Check WHO_AM_I
    tx_data[0] = 0x80 | 0x13; // Read + WHO_AM_I register
    tx_data[1] = 0x00; // Dummy data
    tx_data[2] = 0x00; // Dummy data
    
    accelerometer_tx(tx_data, 3);
    accelerometer_rx_read(rx_data, 3);
    
    if(rx_data[2] != 0x86)
    {
        // Error expected 86h for FXLS8974CF
        while(1)
        {}
    }

    // -------------------------------------------------------------------------
    // Enable active mode
    tx_data[0] = 0x00 | 0x15; // Write + SENS_CONFIG1 register
    tx_data[1] = 0x00; // Dummy data
    tx_data[2] = 0x01; // ACTIVE enable, rest disable
    
    accelerometer_tx(tx_data, 3);
    accelerometer_rx_read(rx_data, 3);

    // -------------------------------------------------------------------------
    // Read temperature
    tx_data[0] = 0x80 | 0x01; // Read + TEMP_OUT register
    tx_data[1] = 0x00; // Dummy data
    tx_data[2] = 0x00; // Dummy data
    
    accelerometer_tx(tx_data, 3);
    accelerometer_rx_read(rx_data, 3);
    
    uint8_t temperature = 25 + (int8_t)(rx_data[2]);

    while(1)
    {
        // Wait for interrupt
        __WFI();
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

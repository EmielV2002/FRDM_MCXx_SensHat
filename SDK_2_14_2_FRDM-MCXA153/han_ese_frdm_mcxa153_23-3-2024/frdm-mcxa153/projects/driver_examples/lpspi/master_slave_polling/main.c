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
#include <string.h>
#include "lpspi0_master.h"
#include "lpspi1_slave.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#define N_DATA (4)

#if (N_DATA < 1) || (N_DATA > 4)
#error "This example supports a datasize of 1, 2, 3 or 4 bytes"
#endif
// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static uint8_t lpspi0_tx_data[N_DATA];
static uint8_t lpspi0_rx_data[N_DATA];

static uint8_t lpspi1_tx_data[N_DATA];
static uint8_t lpspi1_rx_data[N_DATA];

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    lpspi1_slave_init();
    lpspi0_master_init();

    // Prepare transmit buffers
    for(uint8_t i=0; i<N_DATA; i++)
    {
        lpspi0_tx_data[i] = i;
        lpspi1_tx_data[i] = 16 + i;
    }

    while(1)
    {
        // Prepare receive buffers
        memset(lpspi0_rx_data, 0, N_DATA);
        memset(lpspi1_rx_data, 0, N_DATA);

        // LPSPI1: Start receiving by using hardware FIFOs
        //   RXFIFO - stores received characters
        //   TXFIFO - stores characters to be transmitted
        lpspi1_slave_rx_start(lpspi1_tx_data, N_DATA);

        // LPSPI0: transmit
        //   RXFIFO - stores received characters in hardware FIFO
        lpspi0_master_tx(lpspi0_tx_data, N_DATA);

        // LPSPI1: read result from hardware RXFIFO
        lpspi1_slave_rx_read(lpspi1_rx_data, N_DATA);

        // LPSPI0: read result from hardware RXFIFO
        lpspi0_master_rx_read(lpspi0_rx_data, N_DATA);

        // Check the results
        if(memcmp(lpspi0_tx_data, lpspi1_rx_data, N_DATA) != 0)
        {
            // Transmission error from LPSPI0 to LPSPI1
            while(1)
            {}
        }

        if(memcmp(lpspi1_tx_data, lpspi0_rx_data, N_DATA) != 0)
        {
            // Transmission error from LPSPI1 to LPSPI0
            while(1)
            {}
        }

        // Delay
        for(uint32_t i=0; i<10000000; i++)
        {}
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------


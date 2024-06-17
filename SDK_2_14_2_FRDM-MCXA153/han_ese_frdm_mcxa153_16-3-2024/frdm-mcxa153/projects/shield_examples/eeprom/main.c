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
#include "eeprom.h"

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
    eeprom_init();
    
    uint8_t sr = eeprom_rdsr();
    
    if(sr != 0xF0)
    {
        // Unexpected value, reconnect the board and try again
        while(1)
        {}
    }
    
#if 1
    // Read example    

    // Prepare buffers
    for(uint8_t i=0; i<N_DATA; i++)
    {
        tx_data[i] = 0;
        rx_data[i] = 0;
    }
    
    // READ Read from memory array 0000X011 
    tx_data[0] = 0x03; // Instruction
    tx_data[1] = 0x00; // Address
    eeprom_tx(tx_data, N_DATA);
    eeprom_rx_read(rx_data, N_DATA);


#else
    // Write example

    // Enable Write enable latch bit
    eeprom_we(true);
    
    // Prepare buffers
    for(uint8_t i=0; i<N_DATA; i++)
    {
        tx_data[i] = i;
        rx_data[i] = 0;
    }
    
    // Wait while Write In Progress (WIP)
    while(eeprom_wip())
    {}
    
    // WRITE Write to memory array 0000X010
    tx_data[0] = 0x02; // Instruction
    tx_data[1] = 0x00; // Address
    eeprom_tx(tx_data, N_DATA);
    eeprom_rx_read(rx_data, N_DATA);
    
    // Wait while Write In Progress (WIP)
    while(eeprom_wip())
    {}

    // Disable Write enable latch bit
    // Is redundant, because the datasheet states:
    // "The WEL bit (write enable latch) is also reset at the end of the write
    //  cycle t_W"
    eeprom_we(false);
    
#endif

    while(1)
    {
        // Wait for interrupt
        __WFI();
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

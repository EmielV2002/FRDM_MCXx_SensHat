/*! ***************************************************************************
 *
 * \brief     P3T1755 - I3C, I 2 C-bus interface, 0.5 °C accuracy, digital 
 *            temperature sensor
 * \file      p3t1755_polling.c
 * \author    Hugo Arends
 * \date      February 2024
 *
 * \see       NXP. (2024). P3T1755 - I3C, I 2 C-bus interface, 0.5 °C accuracy,
 *            digital temperature sensor - Product data sheet. Rev. 1.1, 
 *            04/01/2023. From:
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
#include "p3t1755_polling.h"
#include "lpi2c0_controller_polling.h"

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
void p3t1755_init(void)
{
    lpi2c0_controller_init();
    
    // Check connectivity by reading the control register. The POR value is 
    // 0x28 (NXP, 2023).
    // 
    uint8_t reg = p3t1755_get_configuration_reg();
    
    if(reg != 0x28)
    {
        // Error
    }
} 

uint8_t p3t1755_get_configuration_reg(void)
{
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK | 
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;
    
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(0b1001000 << 1);
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte select configuration register 
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0b00000001);
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + r
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA((0b1001000 << 1) | 1 );
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}

    // Command: 001b - Receive (DATA[7:0] + 1) bytes. DATA[7:0] is used as a
    //          byte counter.
    // Data   : 0
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b001) | LPI2C_MTDR_DATA(0);
           
    // Wait if the RXFIFO is empty
    while(lpi2c0_rxfifo_empty())
    {}

    // Read the data
    uint8_t data = (uint8_t)LPI2C0->MRDR;
    
    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);
    
    return data;
}

void p3t1755_set_configuration_reg(const uint8_t val)
{
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK | 
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;
    
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(0b1001000 << 1);
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte select configuration register 
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0b00000001);
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}

    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte select configuration register 
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(val);
    
    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);
}

float p3t1755_get_temperature(void)
{
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK | 
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;
    
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(0b1001000 << 1);
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte Temperature register (Temp, read only)
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0b00000000);
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + r
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA((0b1001000 << 1) | 1 );
    
    // Wait if the TXFIFO is full
    while(lpi2c0_txfifo_full())
    {}

    // Command: 001b - Receive (DATA[7:0] + 1) bytes. DATA[7:0] is used as a
    //          byte counter.
    // Data   : 1
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b001) | LPI2C_MTDR_DATA(1);
           
    // Wait if the RXFIFO is empty
    while(lpi2c0_rxfifo_empty())
    {}

    // Read the first byte (MSB)
    uint16_t msb = (uint8_t)LPI2C0->MRDR;

    // Wait if the RXFIFO is empty
    while(lpi2c0_rxfifo_empty())
    {}

    // Read the second byte (LSB)
    uint16_t lsb = (uint8_t)LPI2C0->MRDR;
    
    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);
    
    // Calculate temperature
    uint16_t temp_data = (int16_t)(msb << 4) | (lsb >> 4);
    
    float temperature = 0;
    
    // Positive temperature?
    if((temp_data & 0b0000100000000000) == 0)
    {
        temperature = temp_data * 0.0625f;
    }
    else
    {
        temp_data = (~temp_data) & 0x0FFF;
        temperature = -((temp_data + 1) * 0.0625f);
    }
    
    return temperature;
}

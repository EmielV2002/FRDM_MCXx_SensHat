/*! ***************************************************************************
 *
 * \brief     P3T1755 - I3C, I 2 C-bus interface, 0.5 °C accuracy, digital
 *            temperature sensor
 * \file      p3t1755_interrupt.c
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
#include "p3t1755_interrupt.h"
#include "lpi2c0_controller_interrupt.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
// Statemachine keeping track of update states
typedef enum
{
    STATE_IDLE = 0,
    STATE_UPDATING,
    STATE_UPDATED,
}state_t;
// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint16_t tx_buffer[8];
static volatile uint32_t tx_count = 0;
static volatile uint32_t tx_total = 0;
static volatile uint16_t rx_buffer[8];
static volatile uint32_t rx_count = 0;
static volatile uint32_t rx_total = 0;

static volatile state_t configuration_reg_state = STATE_IDLE;
static volatile state_t temperature_state = STATE_IDLE;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void p3t1755_init(void)
{
    lpi2c0_controller_init();

	// Enable LPI2C0 interrupts
	NVIC_SetPriority(LPI2C0_IRQn, 3);
    NVIC_ClearPendingIRQ(LPI2C0_IRQn);
    NVIC_EnableIRQ(LPI2C0_IRQn);

    // Globally enable interrupts
    __enable_irq();

    // Check connectivity by reading the control register. The POR value is
    // 0x28 (NXP, 2023).
    //
    p3t1755_get_configuration_reg();

    // Wait for result
    uint8_t reg;
    while(!p3t1755_configuration_reg(&reg))
    {}
    
    if(reg != 0x28)
    {
        // Error
    }
}

void p3t1755_get_configuration_reg(void)
{
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();

    // Prevent starting a new update while in the middle of an update
    while(configuration_reg_state == STATE_UPDATING)
    {}
    
    configuration_reg_state = STATE_UPDATING;

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Fill transmit buffer
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //         the address in DATA[7:0]
    // Data   : Slave address + w
    tx_buffer[0] = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(0b1001000 << 1);
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte select configuration register 
    tx_buffer[1] = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0b00000001);
    
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + r
    tx_buffer[2] = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA((0b1001000 << 1) | 1 );
    
    // Command: 001b - Receive (DATA[7:0] + 1) bytes. DATA[7:0] is used as a
    //          byte counter.
    // Data   : 0
    tx_buffer[3] = LPI2C_MTDR_CMD(0b001) | LPI2C_MTDR_DATA(0);

    // Set initial value for variables
    tx_count = 0;
    tx_total = 4;
    rx_count = 0;
    rx_total = 1;

    // Reset fifos
    LPI2C0->MCR |= LPI2C_MCR_RTF(1) | LPI2C_MCR_RRF(1);

    // Enable interrupts
    LPI2C0->MIER |= LPI2C_MIER_RDIE(1) | LPI2C_MIER_TDIE(1);
}

void p3t1755_set_configuration_reg(const uint8_t val)
{
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Fill transmit buffer
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w    
    tx_buffer[0] = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(0b1001000 << 1);
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte select configuration register 
    tx_buffer[1] = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0b00000001);
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Pointer byte select configuration register 
    tx_buffer[2] = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(val);

    // Set initial value for variables
    tx_count = 0;
    tx_total = 3;
    rx_count = 0;
    rx_total = 0;

    // Reset fifo
    LPI2C0->MCR |= LPI2C_MCR_RTF(1);

    // Enable interrupts
    LPI2C0->MIER |= LPI2C_MIER_TDIE(1);
}

void p3t1755_get_temperature(void)
{
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();
    
    // Prevent starting a new update while in the middle of an update
    while(temperature_state == STATE_UPDATING)
    {}
    
    temperature_state = STATE_UPDATING;

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Fill transmit buffer
    tx_buffer[0] = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(0b1001000 << 1);
    tx_buffer[1] = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0b00000000);
    tx_buffer[2] = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA((0b1001000 << 1) | 1 );
    tx_buffer[3] = LPI2C_MTDR_CMD(0b001) | LPI2C_MTDR_DATA(1);

    // Set initial value for variables
    tx_count = 0;
    tx_total = 4;
    rx_count = 0;
    rx_total = 2;

    // Reset fifo
    LPI2C0->MCR |= LPI2C_MCR_RTF(1) | LPI2C_MCR_RRF(1);

    // Enable interrupts
    LPI2C0->MIER |= LPI2C_MIER_RDIE(1) | LPI2C_MIER_TDIE(1);
}

void LPI2C0_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(LPI2C0_IRQn);

    // Transmit interrupt?
    if((LPI2C0->MSR & LPI2C_MSR_TDF_MASK) != 0)
    {
        // Put as many data items in the transmit fifo
        while((tx_count < tx_total) && !lpi2c0_txfifo_full())
        {
            LPI2C0->MTDR = tx_buffer[tx_count++];
        }

        // All data transmitted?
        if(tx_count == tx_total)
        {
            // Disable transmit interrupt
            LPI2C0->MIER &= ~LPI2C_MIER_TDIE(1);
        }
    }

    // Receive interrupt?
    if((LPI2C0->MSR & LPI2C_MSR_RDF_MASK) !=0)
    {
        // Read the data
        rx_buffer[rx_count++] = (uint8_t)LPI2C0->MRDR;

        // All data received?
        if(rx_count == rx_total)
        {
            // Disable receive interrupt
            LPI2C0->MIER &= ~LPI2C_MIER_RDIE(1);

            if(configuration_reg_state == STATE_UPDATING)
            {
                configuration_reg_state = STATE_UPDATED;
            }

            if(temperature_state == STATE_UPDATING)
            {
                temperature_state = STATE_UPDATED;
            }
        }
    }
}

bool p3t1755_temperature(float *temperature)
{
    // Finished receiving data?
    if(temperature_state == STATE_UPDATED)
    {
        // Calculate temperature
        uint16_t temp_data = (int16_t)(rx_buffer[0] << 4) | (rx_buffer[1] >> 4);

        float t = 0;

        // Positive temperature?
        if((temp_data & 0b0000100000000000) == 0)
        {
            t = temp_data * 0.0625f;
        }
        else
        {
            temp_data = (~temp_data) & 0x0FFF;
            t = -((temp_data + 1) * 0.0625f);
        }

        // Copy result
        *temperature = t;

        // Idle state
        temperature_state = STATE_IDLE;

        return true;
    }

    return false;
}

bool p3t1755_configuration_reg(uint8_t *reg)
{
    // Finished receiving data?
    if(configuration_reg_state == STATE_UPDATED)
    {
        // Copy result
        *reg = (uint8_t)(rx_buffer[0]);

        // Idle state
        configuration_reg_state = STATE_IDLE;

        return true;
    }

    return false;
}

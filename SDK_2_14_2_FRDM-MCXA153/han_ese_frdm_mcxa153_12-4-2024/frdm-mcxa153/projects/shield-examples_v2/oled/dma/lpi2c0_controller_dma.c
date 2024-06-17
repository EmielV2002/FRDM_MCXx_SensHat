/*! ***************************************************************************
 *
 * \brief     Low-Power I2C controller DMA
 * \file      lpi2c0_controller_dma.c
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
#include "lpi2c0_controller_dma.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
static inline void lpi2c0_wait_busy(void);
static bool lpi2c0_write(const uint8_t address, const uint8_t data[], 
    const uint32_t n, const uint8_t data_or_cmd);
    
// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile bool dma_done = true;

// This buffer size is specific for this application:
// The SSD1306 requires (128*64)/8 = 1024 bytes.
// The buffer also stores additional commands, such as the generation of the
// start condition, data or command byte, stop condition, etc. For that reason
// 16 additional data items can be stored in the buffer.
static uint16_t tx_buffer[1024 + 16];

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpi2c0_controller_init(void)
{
    // Set clock source
	// MUX: [010] = FRO_HF_DIV
	MRCC0->MRCC_LPI2C0_CLKSEL = MRCC_MRCC_LPI2C0_CLKSEL_MUX(2);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_LPI2C0_CLKDIV = 0;

    // Enable modules and leave others unchanged
    // LPI2C0: [1] = Peripheral clock is enabled
	// PORT1: [1] = Peripheral clock is enabled
    // DMA: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_DMA(1);    
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPI2C0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);

    // Release modules from reset and leave others unchanged
    // LPI2C0: [1] = Peripheral is released from reset
    // PORT1: [1] = Peripheral is released from reset
    // DMA: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_DMA(1);    
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPI2C0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);

    // Configure P1_8 and P1_9
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Input buffer enable
    // MUX: [0011] = Alternative 3
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disable
    // PS:  [0] = n.a.
    PORT1->PCR[8] = PORT_PCR_LK(1) | PORT_PCR_MUX(3) | PORT_PCR_PE(1) | 
        PORT_PCR_PS(1) | PORT_PCR_ODE(1) | PORT_PCR_IBE(1); // LPI2C0_SDA
    PORT1->PCR[9] = PORT_PCR_LK(1) | PORT_PCR_MUX(3) | PORT_PCR_PE(1) | 
        PORT_PCR_PS(1) | PORT_PCR_ODE(1) | PORT_PCR_IBE(1); // LPI2C0_SCL
    
    // From section 36.5 Initialization (NXP, 2024)
    //
    // To initialize the LPI2C controller:
    // 1. Configure Controller Configuration 0 (MCFGR0)–Controller Configuration
    //    3 (MCFGR3) as required by the application.
    // 2. Configure Controller Clock Configuration 0 (MCCR0) and Controller
    //    Clock Configuration 1 (MCCR1) to satisfy the timing requirements of
    //    the I2C mode supported by the application.
    // 3. Enable controller interrupts and DMA requests as required by the
    //    application.
    // 4. Enable the LPI2C controller by writing 1 to MCR[MEN].

    // 1.
    //
    // I2C timing parameters to setup the following specifications (see
    // paragraph 36.6 (NXP, 2024)):
    // I2C mode: FAST
    // Clock frequency: 48 MHz
    // I2C baud rate: 400 kbits/s 
    LPI2C0->MCFGR1 = LPI2C_MCFGR1_PRESCALE(0);
    LPI2C0->MCFGR2 = LPI2C_MCFGR2_FILTSDA(1) | LPI2C_MCFGR2_FILTSCL(1);
    
    // 2.
    //
    // See paragraph 36.6
    LPI2C0->MCCR0 = LPI2C_MCCR0_DATAVD(0x0F) | LPI2C_MCCR0_SETHOLD(0x1D) | 
        LPI2C_MCCR0_CLKHI(0x35) | LPI2C_MCCR0_CLKLO(0x3E);

    // 3.
    //
    // TDDE: [1] = Enables DMA transmit data
    LPI2C0->MDER = LPI2C_MDER_TDDE(1);

    // 4.
    //
    // MEN: [1] = Controller Enable
    LPI2C0->MCR |= LPI2C_MCR_MEN(1);
    
    // Write a single time to this register, or it will be reset!!
    // See 15.6.2.7:
    //     "If there is an attempt to write a mux configuration value that is
    //      already consumed by any channel, a mux configuration of 0 (SRC = 0)
    //      will be written"
    DMA0->CH[0].CH_MUX = DMA_CH_MUX_SRC(12);

    // Enable DMA channel 0 interrupts
	NVIC_SetPriority(DMA_CH0_IRQn, 3);
    NVIC_ClearPendingIRQ(DMA_CH0_IRQn);
    NVIC_EnableIRQ(DMA_CH0_IRQn);    
} 

/*!
 * \brief Sends multiple commands to the Oled display
 *
 * All commands are transferred in a single I2C transfer to the Oled display.
 * If there is no response within the timeout, this function will return false.
 *
 * \param[in]  address  I2C address of the Oled display
 * \param[in]  cmd      Pointer to the array of commands to be transmitted
 * \param[in]  n        Number of commands
 *
 * \return True on successful communication, false otherwise
 */
bool lpi2c0_write_cmd(const uint8_t address, const uint8_t cmd[], const uint32_t n)
{
    return lpi2c0_write(address, cmd, n, 0x00);
}

/*!
 * \brief Sends multiple data bytes to the Oled display
 *
 * All data bytes are transferred in a single I2C transfer to the Oled display.
 * If there is no response within the timeout, this function will return false.
 *
 * \param[in]  address  I2C address of the Oled display
 * \param[in]  data     Pointer to the array of data bytes to be transmitted
 * \param[in]  n        Number of data bytes
 *
 * \return True on successful communication, false otherwise
 */
bool lpi2c0_write_data(const uint8_t address, const uint8_t data[], const uint32_t n)
{
    return lpi2c0_write(address, data, n, 0x40);
}

bool lpi2c0_write(const uint8_t address, const uint8_t data[], const uint32_t n,
    const uint8_t data_or_cmd)
{    
    // Wait as long as bus or controller is busy
    lpi2c0_wait_busy();
    
    // Wait for DMA to complete
    while(dma_done == false)
    {}
    
    dma_done = false;
    
    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK | 
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;
        
    // Copy data to buffer
    uint16_t cnt = 0;
    
    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    tx_buffer[cnt++] = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(address);
    
    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Send control byte: next byte is acted as a data
    tx_buffer[cnt++] = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(data_or_cmd);
    
    for(uint32_t i=0; i<n; ++i)
    {
        // Command: 000b - Transmit the value in DATA[7:0]
        // Data   : Send control byte: next byte is acted as a data
        tx_buffer[cnt++] = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(data[i]);
    }

    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    tx_buffer[cnt++] = LPI2C_MTDR_CMD(0b010);
    
    // From section 15.5.1 eDMA initialization (NXP, 2024)
    //
    // To initialize the eDMA:
    // 1. Write to the MP_CSR if a configuration other than the default is
    //    wanted.
    // 2. Write the channel priority levels to the CHn_PRI registers and group
    //    priority levels to the CHn_GRPRI registers if a configuration other
    //    than the default is wanted.
    // 3. Enable error interrupts in the CHn_CSR[EEI] registers if they are
    //    wanted.
    // 4. Write the 32-byte TCD for each channel that may request service.
    // 5. Enable any hardware service requests via the CHn_CSR[ERQ] registers.
    // 6. Request channel service via either:
    //    - Software: setting TCDn_CSR[START]
    //    - Hardware: slave device asserting its eDMA peripheral request signal

    // 1.
    //
    // Use default configuration

    // 2.
    //
    // Use default configuration

    // 3.
    //
    // No interrupts

    // 4.
    //
    DMA0->CH[0].TCD_SADDR = (uint32_t)tx_buffer;
    DMA0->CH[0].TCD_SOFF = DMA_TCD_SOFF_SOFF(2);
    DMA0->CH[0].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(1) |
                           DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(1);
    DMA0->CH[0].TCD_NBYTES_MLOFFNO = 2;
    //DMA0->CH[0].TCD_NBYTES_MLOFFYES = 2;
    DMA0->CH[0].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(0);
    DMA0->CH[0].TCD_DADDR = (uint32_t)(&(LPI2C0->MTDR));
    DMA0->CH[0].TCD_DOFF = DMA_TCD_DOFF_DOFF(0);
    DMA0->CH[0].TCD_CITER_ELINKNO = cnt;
    //DMA0->CH[0].TCD_CITER_ELINKYES = cnt;
    DMA0->CH[0].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);
    DMA0->CH[0].TCD_CSR = DMA_TCD_CSR_DREQ(1) | DMA_TCD_CSR_INTMAJOR(1);
    DMA0->CH[0].TCD_BITER_ELINKNO = cnt;
    //DMA0->CH[0].TCD_BITER_ELINKYES = cnt;

    // 5.
    //
    DMA0->CH[0].CH_CSR |= DMA_CH_CSR_ERQ(1);

    // 6.
    //
    DMA0->CH[0].TCD_CSR |= DMA_TCD_CSR_START(1);
    
    return true;
}

void DMA_CH0_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(DMA_CH0_IRQn);

    DMA0->CH[0].CH_INT = DMA_CH_INT_INT_MASK;
    
    dma_done = true;
}

static inline void lpi2c0_wait_busy(void)
{
    // Wait as long as Bus Busy Flag or Controller Busy Flag is set
    while((LPI2C0->MSR & (LPI2C_MSR_BBF_MASK | LPI2C_MSR_MBF_MASK)) != 0)
    {}
}

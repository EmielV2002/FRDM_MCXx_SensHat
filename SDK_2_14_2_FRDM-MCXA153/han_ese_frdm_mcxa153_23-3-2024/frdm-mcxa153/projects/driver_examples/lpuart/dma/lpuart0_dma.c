/*! ***************************************************************************
 *
 * \brief     Low-Power UART0 DMA driver
 * \file      lpuart0_dma.c
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
#include "lpuart0_dma.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile bool dma_write_done = true;
static volatile bool dma_read_done = true;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpuart0_dma_init(const uint32_t baudrate)
{
    // Set clock source
	// MUX: [000] = FRO_12M
	MRCC0->MRCC_LPUART0_CLKSEL = 0;

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_LPUART0_CLKDIV = 0;

    // Enable modules and leave others unchanged
    // LPUART0: [1] = Peripheral clock is enabled
	// PORT0: [1] = Peripheral clock is enabled
    // DMA: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_DMA(1);

    // Release modules from reset and leave others unchanged
    // LPUART0: [1] = Peripheral is released from reset
    // PORT0: [1] = Peripheral is released from reset
    // DMA: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_DMA(1);

    // Configure P0_2
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Input buffer enable
    // MUX: [0010] = Alternative 2 - LPUART0_RXD
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT0->PCR[2] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1);

    // Configure P0_3
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [0] = Input buffer disable
    // MUX: [0010] = Alternative 2 - LPUART0_TXD
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT0->PCR[3] = PORT_PCR_LK(1) | PORT_PCR_MUX(2);

    // Configure LPUART0. Although there are a lot of configuration options, the
    // default configuration takes the following steps:
    // 1. Configure baud rate
    // 2. Enable receiver and/or transmitter

    // 1.
    //
    // Configure baud rate
    // OSR: [00111] = Results in an OSR of 8 (7+1)
    // SBR: [.............] = baud rate = baud clock / ((OSR + 1) * SBR)
    //                        => SBR = baud clock / (baud rate * (OSR+1))
    LPUART0->BAUD &= ~(1<<27);
    LPUART0->BAUD |= LPUART_BAUD_SBR(12e6 / (baudrate * 8));

    // 2.
    //
    // TE: [1] = Transmitter Enable
    // RE: [1] = Receiver Enable
    LPUART0->CTRL |= LPUART_CTRL_TE(1) | LPUART_CTRL_RE(1);


    // Write a single time to this register, or it will be reset!!
    // See 15.6.2.7:
    //     "If there is an attempt to write a mux configuration value that is
    //      already consumed by any channel, a mux configuration of 0 (SRC = 0)
    //      will be written"
    DMA0->CH[0].CH_MUX = DMA_CH_MUX_SRC(22);
    DMA0->CH[1].CH_MUX = DMA_CH_MUX_SRC(21);

    // Enable DMA channel 0 interrupts
	NVIC_SetPriority(DMA_CH0_IRQn, 3);
    NVIC_ClearPendingIRQ(DMA_CH0_IRQn);
    NVIC_EnableIRQ(DMA_CH0_IRQn);

    // Enable DMA channel 1 interrupts
	NVIC_SetPriority(DMA_CH1_IRQn, 2);
    NVIC_ClearPendingIRQ(DMA_CH1_IRQn);
    NVIC_EnableIRQ(DMA_CH1_IRQn);
}

void lpuart0_dma_write(const uint8_t *buffer, const uint16_t n)
{
    // Wait
    while(dma_write_done == false)
    {}

    dma_write_done = false;

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
    DMA0->CH[0].TCD_SADDR = (uint32_t)buffer;
    DMA0->CH[0].TCD_SOFF = DMA_TCD_SOFF_SOFF(1);
    DMA0->CH[0].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(0) |
                           DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(0);
    DMA0->CH[0].TCD_NBYTES_MLOFFNO = 1;
    //DMA0->CH[0].TCD_NBYTES_MLOFFYES = 1;
    DMA0->CH[0].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(0);
    DMA0->CH[0].TCD_DADDR = (uint32_t)(&(LPUART0->DATA));
    DMA0->CH[0].TCD_DOFF = DMA_TCD_DOFF_DOFF(0);
    DMA0->CH[0].TCD_CITER_ELINKNO = n;
    //DMA0->CH[0].TCD_CITER_ELINKYES = n;
    DMA0->CH[0].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);
    DMA0->CH[0].TCD_CSR = DMA_TCD_CSR_DREQ(1) | DMA_TCD_CSR_INTMAJOR(1);
    DMA0->CH[0].TCD_BITER_ELINKNO = n;
    //DMA0->CH[0].TCD_BITER_ELINKYES = n;

    // 5.
    //
    DMA0->CH[0].CH_CSR |= DMA_CH_CSR_ERQ(1);

    // 6.
    //
    // TDMAE : [1] = Enables STAT[TDRE] to generate a DMA request
    LPUART0->BAUD |= LPUART_BAUD_TDMAE_MASK;
}

void DMA_CH0_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(DMA_CH0_IRQn);

    DMA0->CH[0].CH_INT = DMA_CH_INT_INT_MASK;
    
    LPUART0->BAUD &= ~LPUART_BAUD_TDMAE_MASK;
    
    dma_write_done = true;
}

bool lpuart0_dma_write_done(void)
{
    return dma_write_done;
}

void lpuart0_dma_read(const uint8_t *buffer, const uint16_t n)
{
    // Wait
    while(dma_read_done == false)
    {}

    dma_read_done = false;

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
    DMA0->CH[1].TCD_SADDR = (uint32_t)(&(LPUART0->DATA));
    DMA0->CH[1].TCD_SOFF = DMA_TCD_SOFF_SOFF(0);
    DMA0->CH[1].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(0) |
                           DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(0);
    DMA0->CH[1].TCD_NBYTES_MLOFFNO = 1;
    //DMA0->CH[1].TCD_NBYTES_MLOFFYES = 1;
    DMA0->CH[1].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(0);
    DMA0->CH[1].TCD_DADDR = (uint32_t)buffer;
    DMA0->CH[1].TCD_DOFF = DMA_TCD_DOFF_DOFF(1);
    DMA0->CH[1].TCD_CITER_ELINKNO = n;
    //DMA0->CH[1].TCD_CITER_ELINKYES = n;
    DMA0->CH[1].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);
    DMA0->CH[1].TCD_CSR = DMA_TCD_CSR_DREQ(1) | DMA_TCD_CSR_INTMAJOR(1);
    DMA0->CH[1].TCD_BITER_ELINKNO = n;
    //DMA0->CH[1].TCD_BITER_ELINKYES = n;

    // 5.
    //
    DMA0->CH[1].CH_CSR |= DMA_CH_CSR_ERQ(1);

    // 6.
    //
    // RDMAE : [1] = Enables STAT[RDRF] to generate a DMA request
    LPUART0->BAUD |= LPUART_BAUD_RDMAE_MASK;
}


void DMA_CH1_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(DMA_CH1_IRQn);

    DMA0->CH[1].CH_INT = DMA_CH_INT_INT_MASK;
    
    LPUART0->BAUD &= ~LPUART_BAUD_RDMAE_MASK;

    dma_read_done = true;
}

bool lpuart0_dma_read_done(void)
{
    return dma_read_done;
}

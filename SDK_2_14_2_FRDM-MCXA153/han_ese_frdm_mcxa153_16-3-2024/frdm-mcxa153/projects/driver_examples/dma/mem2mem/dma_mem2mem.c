/*! ***************************************************************************
 *
 * \brief     DMA memory-to-memory copy
 * \file      dma_mem2mem.h
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
#include "dma_mem2mem.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile bool dma_done = true;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void dma_mem2mem_init(void)
{
    // Enable modules and leave others unchanged
    // DMA: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_DMA(1);

    // Release modules from reset and leave others unchanged
    // DMA: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_DMA(1);

    // Enable DMA channel 0 interrupts
	NVIC_SetPriority(DMA_CH0_IRQn, 3);
    NVIC_ClearPendingIRQ(DMA_CH0_IRQn);
    NVIC_EnableIRQ(DMA_CH0_IRQn);
}

void dma_mem2mem_start(const uint8_t *src, uint8_t *dst, const uint32_t n)
{
    // Wait
    while(dma_done == false)
    {}

    dma_done = false;

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
    // No error interrupts

    // 4.
    //
    DMA0->CH[0].TCD_SADDR = (uint32_t)src;
    DMA0->CH[0].TCD_SOFF = DMA_TCD_SOFF_SOFF(1);
    DMA0->CH[0].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(0) |
                           DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(0);
    DMA0->CH[0].TCD_NBYTES_MLOFFNO = n;
    //DMA0->CH[0].TCD_NBYTES_MLOFFYES = n;
    DMA0->CH[0].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(0);
    DMA0->CH[0].TCD_DADDR = (uint32_t)dst;
    DMA0->CH[0].TCD_DOFF = DMA_TCD_DOFF_DOFF(1);
    DMA0->CH[0].TCD_CITER_ELINKNO = 1;
    //DMA0->CH[0].TCD_CITER_ELINKYES = 1;
    DMA0->CH[0].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);
    DMA0->CH[0].TCD_CSR = DMA_TCD_CSR_DREQ(1) | DMA_TCD_CSR_INTMAJOR(1);
    DMA0->CH[0].TCD_BITER_ELINKNO = 1;
    //DMA0->CH[0].TCD_BITER_ELINKYES = 1;

    // 5.
    //
    // No hardware service requests

    // 6.
    //
    DMA0->CH[0].TCD_CSR |= DMA_TCD_CSR_START(1);
}

void DMA_CH0_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(DMA_CH0_IRQn);
    
    DMA0->CH[0].CH_INT = DMA_CH_INT_INT_MASK;
    
    dma_done = true;
}

bool dma_mem2mem_done(void)
{
    return dma_done;
}

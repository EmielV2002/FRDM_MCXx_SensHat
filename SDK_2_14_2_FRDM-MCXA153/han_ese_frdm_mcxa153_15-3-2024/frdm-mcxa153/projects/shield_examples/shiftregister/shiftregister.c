/*! ***************************************************************************
 *
 * \brief     Shift register
 * \file      shiftregister.c
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
#include "shiftregister.h"
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
static void lpspi1_wait_busy(void);
static bool lpspi1_txfifo_full(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void shiftregister_init(void)
{
    // Set clock source
	// MUX: [000] = FRO_12M
	MRCC0->MRCC_LPSPI1_CLKSEL = MRCC_MRCC_LPSPI1_CLKSEL_MUX(0);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_LPSPI1_CLKDIV = 0;
   
    // Enable modules and leave others unchanged
    // LPSPI1: [1] = Peripheral clock is enabled
	// PORT2: [1] = Peripheral clock is enabled
    // PORT3: [1] = Peripheral clock is enabled
    // GPIO3: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPSPI1(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);

    // Release modules from reset and leave others unchanged
    // LPSPI1: [1] = Peripheral is released from reset
    // PORT2: [1] = Peripheral is released from reset
    // PORT3: [1] = Peripheral is released from reset
    // GPIO3: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPSPI1(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);

    // Configure P3_1, P2_12, P2_13 and P2_16
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Input buffer enable
    // MUX: [0010] = Alternative 2
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT3->PCR[1] = PORT_PCR_LK(1) | PORT_PCR_MUX(0); // GPIO
    PORT2->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_SCK
    PORT2->PCR[13] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_SDO
    PORT2->PCR[16] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_SDI
    
    GPIO3->PDOR |= (1<<1);
    GPIO3->PDDR |= (1<<1);

    // PCSCFG: [0] = PCS[3:2] configured for chip select function
    // OUTCFG: [0] = Retain last value
    // PINCFG: [00] =  SIN is used for input data; SOUT is used for output data
    // MATCFG: [000] = Match is disabled
    // PCSPOL: [0000] = Active low
    // PARTIAL: [0] = n.a.
    // NOSTALL: [0] = Disable stall transfers
    // AUTOPCS: [0] = Disable automatic PCS
    // SAMPLE: [0] = SCK edge
    // MASTER: [1] = Master mode
    LPSPI1->CFGR1 = LPSPI_CFGR1_MASTER(1);

    // SCKPCS: [00000101] = SCK-to-PCS delay of (5+1) cycles
    // PCSSCK: [00000101] = PCS-to-SCK delay of (5+1) cycles
    // DBT: [00000000] = n.a.
    // SCKDIV: [00000000] = n.a.
    LPSPI1->CCR = 0x05050000;

    // SCKSCK: [00000101] = SCK Inter-Frame delay of (5+1) cycles
    // PCSPCS: [00000010] = PCS to PCS delay of (2 + 2 + 2) cycles
    // SCKHLD: [00000101] = SCK hold of (5+1) cycles
    // SCKSET: [00000101] = SCK setup of (5+1) cycles
    LPSPI1->CCR1 = 0x05020505;

    // RRF: [0] = No reset receive FIFO
    // RTF: [0] = No reset transmit FIFO
    // DBGEN: [0] = Disables LPSPI when the CPU is in debug mode
    // RST: [0] = No software Reset
    // MEN: [1] = Module enable
    LPSPI1->CR = LPSPI_CR_MEN(1);
}

void shiftregister_write(uint8_t *buffer, const uint32_t n)
{
    GPIO3->PCOR = (1<<1);
    __ASM("nop");
    GPIO3->PSOR = (1<<1);

    // Wait as long as bus or controller is busy
    lpspi1_wait_busy();

    // Flush FIFOs
    LPSPI1->CR |= LPSPI_CR_RTF(1) | LPSPI_CR_RRF(1);

    // Clear all status flags
    LPSPI1->SR = LPSPI_SR_MBF_MASK | LPSPI_SR_DMF_MASK | LPSPI_SR_REF_MASK |
        LPSPI_SR_TEF_MASK | LPSPI_SR_TCF_MASK | LPSPI_SR_FCF_MASK |
        LPSPI_SR_WCF_MASK | LPSPI_SR_RDF_MASK | LPSPI_SR_TDF_MASK;

    // CPOL: [0] = Clock polarity inactive low
    // CPHA: [0] = Clock polarity captured
    // PRESCALE: [001] = Divide by 2
    // PCS: [00] = Transfer using PCS[0]
    // LSBF: [0] = MSB first
    // BYSW: [0] = Disable byte swap
    // CONT: [1] = Continuous transfer enabled.  In Master mode, this field
    //             keeps PCS asserted at the end of the frame size until a
    //             command word is received that starts a new frame.
    // CONTC: [0] = Command word for start of new transfer
    // RXMSK: [1] = Mask receive data
    // TXMSK: [0] = Normal transfer
    // WIDTH: [00] = 1-bit transfer
    // FRAMESZ: [000000000111] = Frame size = (7+1)
    LPSPI1->TCR = LPSPI_TCR_PRESCALE(0b001) | LPSPI_TCR_CONT(1) | 
        LPSPI_TCR_RXMSK(1) | LPSPI_TCR_FRAMESZ(7);
    
    // Note: In case receiving is not used, set RXMSK [1]

    for(uint32_t i=0; i<n; i++)
    {
        // Wait if the TXFIFO is full
        while(lpspi1_txfifo_full())
        {}

        // Transmit data
        LPSPI1->TDR = buffer[i];
    }

    // Wait if the TXFIFO is full
    while(lpspi1_txfifo_full())
    {}

    // CONT: [1] = Continuous transfer disabled
    LPSPI1->TCR &= ~(LPSPI_TCR_CONT(1));
}

inline void lpspi1_wait_busy(void)
{
    // Wait as long as Bus Busy Flag or Controller Busy Flag is set
    while((LPSPI1->SR & LPSPI_SR_MBF_MASK) != 0)
    {}
}

inline bool lpspi1_txfifo_full(void)
{
    uint32_t n = (LPSPI1->FSR & LPSPI_FSR_TXCOUNT_MASK) >> LPSPI_FSR_TXCOUNT_SHIFT;

    // See reference manual section 38.2.2 Features (NXP, 2024)
    //
    // 4-word transmit and command FIFO
    return n == 4;
}

/*! ***************************************************************************
 *
 * \brief     Low-Power UART0 polling driver
 * \file      lpuart0_polling.c
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
#include "lpuart0_interrupt.h"
#include "fifo.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static fifo_t tx;
static fifo_t rx;
static uint8_t tx_buffer[128];
static uint8_t rx_buffer[128];

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpuart0_init(const uint32_t baudrate)
{
    // Initialize FIFOs
    f_init(&tx, tx_buffer, sizeof(tx_buffer));
    f_init(&rx, rx_buffer, sizeof(rx_buffer));

    // Set clock source
	// MUX: [010] = FRO_HF_DIV (defaults: FRO_HF = 48 MHz; DIV = 1)
	MRCC0->MRCC_LPUART0_CLKSEL = MRCC_MRCC_LPUART0_CLKSEL_MUX(0b010);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
	// DIV: [0000] = divider value = (DIV+1) = 1
	MRCC0->MRCC_LPUART0_CLKDIV = 0;

    // Enable modules and leave others unchanged
    // LPUART0: [1] = Peripheral clock is enabled
	// PORT0: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

    // Release modules from reset and leave others unchanged
    // LPUART0: [1] = Peripheral is released from reset
    // PORT0: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT0(1);

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
    // OSR: [01111] = Results in an OSR of 16 (15+1)
    // SBR: [.............] = baud rate = baud clock / ((OSR + 1) * SBR)
    //                        => SBR = baud clock / (baud rate * (OSR+1))
    LPUART0->BAUD = LPUART_BAUD_OSR(0b01111) | 
       LPUART_BAUD_SBR(CLK_FRO_48MHZ / (baudrate * 16));

    // 2.
    //
    // TE: [1] = Transmitter Enable
    // TIE: [0] = Transmitter Disable
    // RE: [1] = Receiver Enable
    // RIE: [1] = Receiver Enable
    LPUART0->CTRL |= LPUART_CTRL_TE(1) | LPUART_CTRL_RIE(1) | LPUART_CTRL_RE(1);

	// Enable LPUART0 interrupts
	NVIC_SetPriority(LPUART0_IRQn, 3);
    NVIC_ClearPendingIRQ(LPUART0_IRQn);
    NVIC_EnableIRQ(LPUART0_IRQn);

    // Globally enable interrupts
    __enable_irq();
}

void lpuart0_putchar(const int data)
{
    // Wait for space to open up
    while(!f_push(&tx, (uint8_t)data))
    {}

    // Enable TDRE interrupt
    LPUART0->CTRL |= LPUART_CTRL_TIE(1);
}

int lpuart0_getchar(void)
{
    uint8_t c=0;

    // Wait for data.
    // If waiting is not desired, call the function lpuart0_rxqsize() first to
    // make sure data is available.
    while(!f_pop(&rx, &c))
    {}

    return (int)c;
}

uint32_t lpuart0_rxcnt(void)
{
    return f_cnt(&rx);
}

void LPUART0_IRQHandler(void)
{
	uint8_t c;

	// Clear the interrupt
	NVIC_ClearPendingIRQ(LPUART0_IRQn);

	// Data transmitted?
	if((LPUART0->STAT & LPUART_STAT_TDRE_MASK) != 0)
	{
        // Send another character?
        if(f_pop(&tx, &c))
        {
        	LPUART0->DATA = c;
        }
        else
        {
            // FIFO is empty so disable TDRE interrupt
            LPUART0->CTRL &= ~(LPUART_CTRL_TIE(1));
        }
	}

	// Data received?
	if((LPUART0->STAT & LPUART_STAT_RDRF_MASK) != 0)
	{
		// Read data
        c = (uint8_t)(LPUART0->DATA);

        // Put in receive FIFO
        if(!f_push(&rx, c))
        {
            // Error: receive FIFO full!!
            // Should not happen, so freeze the system. Update FIFO size to
            // match your application.
            while (1)
            {}
        }
	}
}

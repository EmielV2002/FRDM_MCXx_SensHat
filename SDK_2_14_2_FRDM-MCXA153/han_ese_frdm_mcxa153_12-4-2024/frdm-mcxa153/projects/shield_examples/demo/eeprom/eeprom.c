/*! ***************************************************************************
 *
 * \brief     Low level driver for the Low Power Serial Peripheral Interface
 *            (LPSPI) in master mode controlling an M950x0-R EEPROM
 * \file      eeprom.c
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
 ******************************************************************************/
#include "eeprom.h"
#include "helper_functions/fifo.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#define N_DATA (4+2)

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
void lpspi1_wait_busy(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static fifo_t tx;
static fifo_t rx;
static uint8_t tx_buffer[128];
static uint8_t rx_buffer[128];
static uint8_t tx_data_N[N_DATA];
static uint8_t rx_data_N[N_DATA];
// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void eeprom_init(void)
{
    // Initialize FIFOs
    f_init(&tx, tx_buffer, sizeof(tx_buffer));
    f_init(&rx, rx_buffer, sizeof(rx_buffer));

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

    // Configure P3_11, P2_12, P2_13 and P2_16
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
    PORT3->PCR[11] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_PCS0
    PORT2->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_SCK
    PORT2->PCR[13] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_SDO
    PORT2->PCR[16] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1); // LPSPI1_SDI


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
    
	  // Enable LPSPI1 interrupts
	  NVIC_SetPriority(LPSPI1_IRQn, 1);
    NVIC_ClearPendingIRQ(LPSPI1_IRQn);
    NVIC_EnableIRQ(LPSPI1_IRQn);    
}

void eeprom_tx(uint8_t *buffer, const uint32_t n)
{
    // Copy data to FIFO
    for(uint32_t i=0; i<n; i++)
    {
        f_push(&tx, buffer[i]);
    }
    
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
    // PRESCALE: [000] = Divide by 1
    // PCS: [00] = Transfer using PCS[0]
    // LSBF: [0] = MSB first
    // BYSW: [0] = Disable byte swap
    // CONT: [1] = Continuous transfer enabled.  In Master mode, this field
    //             keeps PCS asserted at the end of the frame size until a
    //             command word is received that starts a new frame.
    // CONTC: [0] = Command word for start of new transfer
    // RXMSK: [0] = Normal transfer
    // TXMSK: [0] = Normal transfer
    // WIDTH: [00] = 1-bit transfer
    // FRAMESZ: [000000000111] = Frame size = (7+1)
    LPSPI1->TCR = LPSPI_TCR_PRESCALE(0b000) | LPSPI_TCR_CONT(1) | LPSPI_TCR_FRAMESZ(7);
    
    // Note: In case receiving is not used, set RXMSK [1]

    // Enable transmit interrupt
    LPSPI1->IER = LPSPI_IER_RDIE(1) | LPSPI_IER_TDIE(1);
}

void eeprom_rx_read(uint8_t *buffer, const uint32_t n)
{
    for(uint32_t i=0; i<n; i++)
    {
        // Wait if the FIFO is empty
        while(f_cnt(&rx) == 0)
        {}
    
        f_pop(&rx, &buffer[i]);
    }
}

inline void lpspi1_wait_busy_eeprom(void) //<---- lpspi1_wait_busy
{
    // Wait as long as Bus Busy Flag or Controller Busy Flag is set
    while((LPSPI1->SR & LPSPI_SR_MBF_MASK) != 0)
    {}
}

void LPSPI1_IRQHandler_eeprom(void) //<---- LPSPI1_IRQHandler
{
    uint8_t c;
    
	// Clear the interrupt
	NVIC_ClearPendingIRQ(LPSPI1_IRQn);

	// Data transmitted?
	if((LPSPI1->SR & LPSPI_SR_TDF_MASK) != 0)
	{
        // Send another character?
        if(f_pop(&tx, &c))
        {
        	LPSPI1->TDR = c;
        }
        else
        {
            // CONT: [1] = Continuous transfer disabled
            LPSPI1->TCR &= ~(LPSPI_TCR_CONT(1));

            // FIFO is empty so disable TDRE interrupt
            LPSPI1->IER &= ~LPSPI_IER_TDIE_MASK;
        }
	}

	// Data received?
	if((LPSPI1->SR & LPSPI_SR_RDF_MASK) != 0)
	{
		// Read data
        c = (uint8_t)(LPSPI1->RDR);

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

uint8_t eeprom_rdsr(void)
{
    // RDSR Read Status register 0000 X101
    uint8_t tx_data[2] = {0x05, 0x00};
    uint8_t rx_data[2] = {0x00, 0x00};
    
    eeprom_tx(tx_data, sizeof(tx_data));
    eeprom_rx_read(rx_data, sizeof(rx_data));

    return rx_data[1];
}

bool eeprom_wip(void)
{
    uint8_t sr = eeprom_rdsr();
    
    return (sr & 0x01) != 0;
}

void eeprom_we(bool wel)
{
    // WREN Write enable 0000X110
    // WRDI Write disable 0000X100

    uint8_t tx_data[1] = {wel ? 0x06 : 0x04};
    uint8_t rx_data[1] = {0x00};
    
    eeprom_tx(tx_data, sizeof(tx_data));
    eeprom_rx_read(rx_data, sizeof(rx_data));
}

int eeprom_main(void)
{
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
      tx_data_N[i] = 0;
      rx_data_N[i] = 0;
  }
    
  // READ Read from memory array 0000X011 
  tx_data_N[0] = 0x03; // Instruction
  tx_data_N[1] = 0x00; // Address
  eeprom_tx(tx_data_N, N_DATA);
  eeprom_rx_read(rx_data_N, N_DATA);

  #else
  // Write example

  // Enable Write enable latch bit
  eeprom_we(true);
     
  // Prepare buffers
  for(uint8_t i=0; i<N_DATA; i++)
  {
        tx_data_N[i] = i;
        rx_data_N[i] = 0;
  }
     
  // Wait while Write In Progress (WIP)
  while(eeprom_wip())
  {}
      
  // WRITE Write to memory array 0000X010
  tx_data_N[0] = 0x02; // Instruction
  tx_data_N[1] = 0x00; // Address
  eeprom_tx(tx_data_N, N_DATA);
  eeprom_rx_read(rx_data_N, N_DATA);
      
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

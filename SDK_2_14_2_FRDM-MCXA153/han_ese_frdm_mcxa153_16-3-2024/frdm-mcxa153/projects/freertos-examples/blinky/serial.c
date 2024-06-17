/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Updated by Hugo Arends February 2024 as a demo for the FRDM-MCXA153 board
 * HAN University of Applied Sciences
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
	BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER FOR UART0.
*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* Library includes. */
#include "MCXA153.h"

/* Demo application includes. */
#include "serial.h"

/*---------------------------------------------------------------------------*/

/* Misc defines. */
#define serINVALID_QUEUE    ( ( QueueHandle_t ) 0 )
#define serNO_BLOCK		    ( ( TickType_t ) 0 )
#define serTX_BLOCK_TIME    ( 40 / portTICK_PERIOD_MS )

/*---------------------------------------------------------------------------*/

/* The queues used to hold characters. */
static QueueHandle_t xRxedChars;
static QueueHandle_t xCharsForTx;
static SemaphoreHandle_t xStringMutex;

/*---------------------------------------------------------------------------*/

/*
 * See the serial.h header file.
 */
portBASE_TYPE xSerialPortInit( unsigned long ulWantedBaud,
                               unsigned portBASE_TYPE uxQueueLength )
{
    portBASE_TYPE xReturn = pdTRUE;

    // Create the queues used to hold Rx/Tx characters.
	xRxedChars = xQueueCreate( uxQueueLength, sizeof(char) );
	xCharsForTx = xQueueCreate( uxQueueLength + 1, sizeof(char) );

	// Create mutex
	xStringMutex = xSemaphoreCreateMutex();

	// If the queue was created correctly then setup the UART peripheral
	if( ( xRxedChars != serINVALID_QUEUE ) && ( xCharsForTx != serINVALID_QUEUE ) )
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
        // OSR: [00111] = Results in an OSR of 8 (7+1)
        // SBR: [.............] = baud rate = baud clock / ((OSR + 1) * SBR)
        //                        => SBR = baud clock / (baud rate * (OSR+1))
        LPUART0->BAUD &= ~(1<<27);
        LPUART0->BAUD |= LPUART_BAUD_SBR(12e6 / (ulWantedBaud * 8));

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
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
}

/*---------------------------------------------------------------------------*/

portBASE_TYPE xSerialPutChar( char cOutChar, TickType_t xBlockTime )
{
    portBASE_TYPE xReturn;

    // Send the character to the queue. Return false if xBlockTime expires.
    if( xQueueSend( xCharsForTx, &cOutChar, xBlockTime ) == pdPASS )
    {
        xReturn = pdPASS;
        LPUART0->CTRL |= LPUART_CTRL_TIE(1);
    }
    else
    {
        xReturn = pdFAIL;
    }

    return xReturn;
}

/*---------------------------------------------------------------------------*/

portBASE_TYPE xSerialGetChar( char *pcRxedChar, TickType_t xBlockTime )
{
	// Get the next character from the buffer.  Return false if no characters
	// are available, or arrive before xBlockTime expires.
	if( xQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}

/*---------------------------------------------------------------------------*/

void vSerialPutString( const char * const pcString )
{
    // NOTE: This implementation does not handle the queue being full as no
    // block time is used!

    const char * pxNext = pcString;

    // Attempt to take the mutex, blocking indefinitely to wait for the mutex
    // if it is not available straight away. The call to xSemaphoreTake() will
    // only return when the mutex has been successfully obtained, so there is
    // no need to check the function return value. If any other delay period
    // was used then the code must check that xSemaphoreTake() returns pdTRUE
    // before accessing the shared resource (which in this case is standard
    // out). As noted earlier in this book, indefinite time outs are not
    // recommended for production code.
    xSemaphoreTake(xStringMutex, portMAX_DELAY);
    {
        while(*pxNext)
        {
            xSerialPutChar(*pxNext, serNO_BLOCK);
            pxNext++;
        }
    }
    xSemaphoreGive(xStringMutex);
}

/*---------------------------------------------------------------------------*/

void LPUART0_IRQHandler( void )
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    char cChar;

	if( (LPUART0->STAT & LPUART_STAT_TDRE_MASK) != 0 )
	{
		// The interrupt was caused by the data register becoming empty.
		// Are there any more characters to transmit?
		if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
		{
			// A character was retrieved from the transmit queue so send it.
			LPUART0->DATA = cChar;
		}
		else
		{
		    // No more characters in the transmit queue, disable transmit
		    // interrupt.
			LPUART0->CTRL &= ~(LPUART_CTRL_TIE(1));
		}
	}

	if( (LPUART0->STAT & LPUART_STAT_RDRF_MASK) != 0 )
	{
        // The interrupt was caused by incoming data. Read the data and store
	    // in the receive queue.
		cChar = (char)LPUART0->DATA;
		xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );
	}

    // Pass the xHigherPriorityTaskWoken value into portEND_SWITCHING_ISR(). If
	// xHigherPriorityTaskWoken was set to pdTRUE inside one of the ...FromISR()
	// functions then calling portEND_SWITCHING_ISR() will request a context switch.
	// If xHigherPriorityTaskWoken is still pdFALSE then calling
	// portEND_SWITCHING_ISR() will have no effect.
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

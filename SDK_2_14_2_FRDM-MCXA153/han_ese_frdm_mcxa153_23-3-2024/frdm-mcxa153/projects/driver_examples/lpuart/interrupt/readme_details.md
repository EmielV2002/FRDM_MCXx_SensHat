# Low-Power UART - interrupt

## Goal

To understand what it takes to use the LPUART with interrupts when compared to polling.

## Interrupts instead of polling

Using LPUART transmitter and receiver interrupts is actually pretty straight forward, as soon as polling has been implemented:

1. Enable the interrupts in the LPUART0->CTRL register.
2. Enable LPUART0 interrupts in the NVIC and set priority.
3. Implement an interrupt handler called LPUART0_IRQHandler().

The hard part is to implement a data structure to:

- Temporary store incoming data
- Temporary store outgoing data

## Circular buffer

One way to implement a data structure for storing data is a [circular buffer](https://en.wikipedia.org/wiki/Circular_buffer). One advantage is that it uses static memory, of which the size is set at compile time. This makes it a fast implementation, when for example, compared to a dynamically allocated singly linked list.

To keep a clear overview and for being able to use the circular buffer for other modules as well, the circular buffer is implemented in the files *fifo.c* and *fifo.h*.

## Putting a character

Transmitter interrupts are disabled by default. Putting a character takes the following steps:

1. Append the character to the transmit fifo.
2. Enable transmit interrupts.
3. Wait for the interrupt to occur. In the interrupt handler, read data from the transmit fifo and transmit this data via the LPUART0->DATA register.
4. If there is no more data, disable transmit interrupts.

*`Q1` Examine the file lpuart0_interrupt.c. What will happen if the transmit fifo is full?*

## Getting a character

Receive interrupts are enabled by default. Getting a character takes the following steps:

1. Wait for an interrupt to occur. In the interrupt handler, read data from the LPUART0->DATA register and put it in the receive fifo.
2. Provide a function to check if data is available in the receive fifo.
3. If data is read, return the first item from the fifo.

*`Q2` Examine the file lpuart0_interrupt.c. What will happen if the receive fifo is full?*

## Final assignment

- Change the baud rate to 115200 and verify that the application is still operational as before.
- *`Q3` At 115200-8n1, how long does it take to transmit 1024 bytes?*

## Extra assignment

Implement a driver to support LPUART2. Proceed as follows:

- Copy the file lpuart0_interrupt.h and rename it to lpuart2_interrupt.h.
- Copy the file lpuart0_interrupt.c and rename it to lpuart2_interrupt.c.
- Add both files to the project.
- Rewrite all functions and function prototypes to support LPUART2. Configure the pins as follows:
    - LPUART2_RXD P1_4
    - LPUART2_TXD P1_5
- Connect a USB-to-UART convertor or a Bluetooth module for testing and verification.

## Answers

### Q1

See the function lpuart0_putchar():

```C
void lpuart0_putchar(const int data)
{
    // Wait for space to open up
    while(!f_push(&tx, (uint8_t)data))
    {}

    // Enable TDRE interrupt
    LPUART0->CTRL |= LPUART_CTRL_TIE(1);
}
```
 
### Q2

See the interrupt handler LPUART0_IRQHandler():

```C
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
```

### Q3

```C
// 115200 bps: 1/115200 s per bit
// 8n1: 1 start bit + 8 data bits + no parity + 1 stop bit = 10 bits per frame
// Total = 1024 * 10 * 1/115200 = 0.091 s
```

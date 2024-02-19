# Low-Power UART - polling

## Goal

To know how to configure the LPUART for basic serial communication and use it by polling status bits.

## Required hardware

- FRDM-MCXA153A board

## LPUART0 features

The MCXA153 has three instances of the LPUART module. These are called LPUART0, LPUART1 and LPUART2. The LPUART is described in the [reference manual chapter 37](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#37%20Low%20Power%20Universal%20Asynchronous%20Receiver/Transmitter%20(LPUART)).

The MCU-Link on the FRDM-MCXA153 supports the VCOM serial port feature, which adds a serial COM port on the host computer and connects it to the target MCU while working as a USB-to-UART bridge. Refer to [board schematic page 6](./../../docs/datasheets/SPF-90829_A1.pdf#page=6).

`?` What LPUART and microcontroller pins are connected to the MCU-Link?

<details>
<summary>Solution</summary>
```C
// LPUART0
// P0_2 - LPUART0_RXD
// P0_3 - LPUART0_TXD
```
</details><br>

`?` The PORT module must be used to configure the function for a pin. Refer to [reference manual paragraph 10.3](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#10.3%20Pinmux%20view) to find the alternative function that must be selected for both these pins.

<details>
<summary>Solution</summary>
```C
// P0_2 - LPUART0_RXD - ALT2
// P0_3 - LPUART0_TXD - ALT2
```
</details><br>

## LPUART configuration

There are a lot of configuration options for he LPUART, however, configuring the LPUART for straight forward 8 data bit, no parity and 1 stop bit communication, is fairly simple.

The steps to take are:

1. Configure clock source in the MRCC module
2. Enable modules in the MRCC module
3. Configure alternate function in the PORT module
4. Set baud rate (based on clock source) in the LPUART module
5. Enable receiver and/or transmitter in the LPUART module

### 1. Configure clock source

```C
// Set clock source
// MUX: [000] = FRO_12M
MRCC0->MRCC_LPUART0_CLKSEL = 0;

// HALT: [0] = Divider clock is running
// RESET: [0] = Divider isn't reset
// DIV: [0000] = divider value = (DIV+1) = 1
MRCC0->MRCC_LPUART0_CLKDIV = 0;
```

### 2. Enable modules

```C
// Enable modules and leave others unchanged
// LPUART0: [1] = Peripheral clock is enabled
// PORT0: [1] = Peripheral clock is enabled
MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

// Release modules from reset and leave others unchanged
// LPUART0: [1] = Peripheral is released from reset
MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);
```

### 3. Configure alternate function

```C
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
```

### 4. Set baud rate

The baud rate is set in the LPUART->BAUD register by the OSR and SBR bit fields. Find the description of this bit field in the reference manual.

`?` What formula is applicable for calculating the *baud rate*?

<details>
<summary>Solution</summary>
```C
// baud rate = baud clock / ((OSR + 1) * SBR)
//
// Alternatively: SBR = baud clock / (baud rate * (OSR+1))
```
</details><br>

`?` What is the value of *baud clock*?

<details>
<summary>Solution</summary>
```C
// As set by in the MRCC: FRO_12M with DIV = 1: 12 MHz
```
</details><br>

`?` What is the reset value of *OSR*?

<details>
<summary>Solution</summary>
```C
// OSR = [01111] = Results in an OSR of 16 (15+1)
```
</details><br>

12 MHz *baud clock* is not a very high frequency, so the OSR will be reduced to 8 by setting it to 0b00111. The following instruction can than be used to configure the baud rate to 9600 bps:

```C
// Configure baud rate of 9600 bps
// OSR: [00111] = Results in an OSR of 8 (7+1)
// SBR: [.............] = baud rate = baud clock / ((OSR + 1) * SBR)
//                        => SBR = baud clock / (9600 * (OSR+1))
LPUART0->BAUD &= ~(1<<27);
LPUART0->BAUD |= LPUART_BAUD_SBR(12e6 / (9600 * 8));
```

### 5. Enable receiver and/or transmitter

The receiver and transmitter are enable in the LPUART0->CTRL register.

`?` Give the instruction(s) to enable receiver and transmitter.

<details>
<summary>Solution</summary>
```C
// TE: [1] = Transmitter Enable
// RE: [1] = Receiver Enable
LPUART0->CTRL |= LPUART_CTRL_TE(1) | LPUART_CTRL_RE(1);
```
</details><br>

## Putting a character

A polling function for putting a character is implemented by checking the Transmit Data Register Empty Flag (TDRE).

`?` Implement a while-loop that waits as long as the transmit data register is empty.

<details>
<summary>Solution</summary>
```C
while((LPUART0->STAT & LPUART_STAT_TDRE_MASK) == 0)
{}
```
</details><br>

## Getting a character

A polling function for getting a character is implemented by checking the Receive Data Register Full Flag (RDRF).

`?` Implement a while-loop that waits as long as the receive data register is not full.

<details>
<summary>Solution</summary>
```C
while((LPUART0->STAT & LPUART_STAT_RDRF_MASK) == 0)
{}
```
</details><br>

## Test and verification

- Open the project lpuart0_polling.
- Build and run the application.
- Open the VCOM (9600-8n1) in a terminal application of your choice.
- Verify that after microcontroller reset a message appears in the terminal application.
- Verify that typing the characters 'r', 'g' and 'b' in the terminal application toggles the corresponding RBG LED.

## Final assignment

None.

## Extra: retarget IO

The functions *lpuart0_putchar()* and *lpuart0_getchar()* can be used for sending and receiving individual characters. It would, however, be very convenient to have the stdio functions for formatting input and output, such as *printf()*. This is possible and is called *retargeting IO*.

The NEWLIB_NANO C standard library](https://en.wikipedia.org/wiki/Newlib), which is included by the linker in the given projects, uses two functions for reading and writing the actual characters. These functions can be replaced by our own functions for reading and writing characters, such as *lpuart0_putchar()* and *lpuart0_getchar()*.

- In MDK-ARM these functions are:
```C
int stdout_putchar(int ch);
int stdin_getchar(void);
```
- In GCC_ARM these functions are:
```C
int _write(int fd, const void *buf, size_t count);
int _read(int fd, const void *buf, size_t count);
```

To keep the projects nice and clean, all these functions have been implemented in the file retarget.c. Notice the use of the defines *__CC_ARM* and *\__NEWLIB__* for conditional compilation for both compilers.

<details>
<summary>Show me!</summary>
```C
#ifdef __CC_ARM

// Functions for redirecting standard output to LPUART0 for MDK-ARM.

int stdout_putchar(int ch)
{
    lpuart0_putchar(ch);
    return ch;
}

int stdin_getchar(void)
{
    return lpuart0_getchar();
}

#endif

#ifdef __NEWLIB__

// Functions for redirecting standard output to LPUART0 for GCC ARM

int _write(int fd, const void *buf, size_t count)
{
	(void)fd;

	for(size_t i=0; i<count; i++)
	{
		lpuart0_putchar(((int *)buf)[i]);
	}

	return count;
}

int _read(int fd, const void *buf, size_t count)
{
	(void)fd;

	for(size_t i=0; i<count; i++)
	{
		((int *)buf)[i] = lpuart0_getchar();
	}

	return count;
}

#endif

```
</details><br>

*Note 1: If the application is built using newlib-nano, by default, floating point format strings (%f) are not supported. To enable this support, -u _printf_float mst be added to the linker.*

*Note 2: In general, stdio printing (such as printf()), should not be performed in an interrupt handler!*


# GPIO input - interrupt

## Goal

Know how to initialize and read a digital input pin by using interrupts in the GPIO module.

## Initialization

It takes the following steps to configure P1_7 for GPIO:

1. Enable the PORT1 and GPIO1 modules in the MRCC module.
2. Initialize P1_7 for GPIO function in the PORT1 module.

    No additional action is needed, because the default direction of P1_7 is input.
    These steps are exactly the same when compared to the [polling](./../input_polling/readme_details.md) example.

    Using interrupts on a GPIO input pin requires two additional steps:

3. Enable the desired interrupt in the GPIO module.
4. Enable GPIO1 interrupts in the NVIC module.

### 1. Enable the PORT1 and GPIO1 modules

```C
// Enable modules and leave others unchanged
// PORT1: [1] = Peripheral clock is enabled
// GPIO1: [1] = Peripheral clock is enabled
MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO1(1);

// Release modules from reset and leave others unchanged
// PORT1: [1] = Peripheral is released from reset
// GPIO1: [1] = Peripheral is released from reset
MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO1(1);
```

### 2. Initialize P1_7 for GPIO function

```C
// Configure pin P1_7
// LK : [1] = Locks this PCR
// INV: [0] = Does not invert
// IBE: [1] = Input Buffer Enable
// MUX: [0000] = Alternative 0 (GPIO)
// DSE: [0] = low drive strength is configured on the corresponding pin,
//            if the pin is configured as a digital output
// ODE: [0] = Disables
// SRE: [0] = Fast
// PE:  [0] = Disables
// PS:  [0] = n.a.
PORT1->PCR[7] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);
```

### 3. Enable the desired interrupt in the GPIO module.

The GPIO module has an Interrupt Control Register (ICR) for each pin. These registers are organized in an array, so the ICR register for P1_7 is used as follows:

```C
GPIO1->ICR[7]
```

This register has two bit fields:

- ISF
- IRQC

Find these ICR register description in the [reference manual](./../../../../docs/datasheets/MCXAP64M96FS3RM.pdf).

*`Q1` What does ISF stand for?*

*`Q2` The ISF bit field shows the text W1C. What does W1C mean?*

*`Q3` What C instruction can be used to clear the ISF bit?*

*`Q4` What other register in the GPIO module can be used to check the interrupt status of P1_7?*

The IRQC bit field contains 4 bits, giving a total of 16 configuration options.

*`Q5` What IRQC configuration option is selected out of reset?*

Configuring P1_7 for setting the ISF and generating an interrupt on falling edges is done as follows:

```C
// ISF: [1] = Clear the flag
// IRQC : [1010] = ISF and interrupt on falling edge
GPIO1->ICR[7] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);
```

### 4. Enable GPIO1 interrupts in the NVIC module.

Enabling interrupts for GPIO1 in the NVIC:

```C
// Enable GPIO1 interrupts
NVIC_SetPriority(GPIO1_IRQn, 3);    
NVIC_ClearPendingIRQ(GPIO1_IRQn);
NVIC_EnableIRQ(GPIO1_IRQn);
```

## Interrupt handler

The interrupt handler for GPIO handles the interrupt request.
The following must be done:

1. Clear the interrupt in the NVIC.
2. The same interrupt handler is executed for all GPIO1 pins. Check to make sure the interrupt was triggered by P1_7 (although strictly speaking, this is not necessary in this example).
3. Clear the interrupt in the GPIO module.
4. Handle the event.

The result is as follows:

```C
void GPIO1_IRQHandler(void)
{
	// Clear the interrupt
	NVIC_ClearPendingIRQ(GPIO1_IRQn);

    // Interrupt handler triggered by P1_7?
    if((GPIO1->ISFR[0] & GPIO_ISFR_ISF7(1)) != 0)
    {
        // Clear the flag
        GPIO1->ISFR[0] = GPIO_ISFR_ISF7(1);

        // Handle the event
        cnt++;
    }
}
```

Note the use of a global variable called **cnt**. This variable is incremented on every interrupt generation. This is a global variable declared as follows:

```C
static volatile uint32_t cnt = 0;
```

This variable is declared ```static```, because the scope is limited to this file.

This variable is declared ```volatile```, because the variable can change outside normal program flow (because it is used in an ISR). The compiler should not optimize any read/write operations.

## Final assignment

Change this example so it uses [SW2](https://docs.nxp.com/bundle/UM12012/page/topics/Push_buttons.html) and the blue RGB LED.

Some tips:

- Find the pin that is connected to SW2 in the [board schematic](./../../../../docs/datasheets/SPF-90829_A1.pdf).
- Another GPIO module also requires another interrupt handler.


## Answers

### Q1

Interrupt Status Flag

Indicates whether the configured interrupt is detected. The pin interrupt configuration is valid in all digital pin muxing modes.

### Q2

See reference manual paragraph 1.5.4.

> Write 1 to clear (w1c)

### Q3

```C
GPIO1->ICR[7] = GPIO_ICR_ISF(1);
```

### Q4

See reference manual paragraph 12.7.1.15

The Interrupt Status Flag Register 0 (ISFR0)

### Q5

Out of reset, the IRQC bit field is 0b0000. Checking the IRQC Interrupt Configuration description in the table shows that:

> 0000b - ISF is disabled

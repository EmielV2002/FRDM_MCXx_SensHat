# General Purpose Input

## Goal

Turn the blue RGB LED on when pressing switch [SW3](https://docs.nxp.com/bundle/UM12012/page/topics/Push_buttons.html).

## Required hardware

- FRDM-MCXA153A board

## Which pin to use?

Before being able to read the logic state of SW3, the physical connection on the FRDM-MCXA153 board must be checked. Refer to the [board schematic page 7](./../../docs/datasheets/SPF-90829_A1.pdf#page=7).

`?` What microcontroller pin is connected to switch SW3 (a.k.a. WAKEUP)?

<details>
<summary>Solution</summary>
```C
// SW3 (WAKEUP): PORT1 pin 7 (P1_7)
```
</details><br>

The schematic also shows that an external pullup resistor is connected to SW3. Pressing SW3 connects it to GND, reading logic 0.

## Initialization

Similar to GPIO output pins, the GPIO modules must be used to read the logic state of a microcontroller pin. However, a pin must first be configured for the GPIO function. Using P1_7 as an example, it takes the following steps to configure the pin for GPIO:

1. Enable the PORT1 and GPIO1 modules in the MRCC module.
2. Initialize P1_7 for GPIO function in the PORT1 module.

No additional action is needed, because the default direction of a pin is input.

### 1. Enable the PORT1 and GPIO1 modules

The MRCC module must be used to enable other modules, such as PORT1 and GPIO1. How this is done is described in the [reference manual paragraph 14.3](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#14.3%20Functional%20description).

```C
// Before a module can be used, its clocks must be enabled (CC != 00) and it
// must be released from reset (MRCC_GLB_RST [peripherals name] = 1). If a
// module is not released from reset (MRCC_GLB_RST [peripherals name] = 0),
// an attempt to access a register within that module is terminated with a
// bus error.
```

This description translates to the following C instructions:

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

Notice the use of defines such as MRCC_MRCC_GLB_CC0_PORT1(1). Using such defines for bit fields makes the code very readable and portable. The naming convention for such defines is as follows:

```C
<MODULE>_<REGISTER>_<BIT FIELD NAME>(x)
```

Want to know how this define works and what other defines are available? [Read this!](./../../docs/cmsis.md)

### 2. Initialize P1_7 for GPIO function

The PORT module must be used to configure the function for a pin. How this is done is described in the [reference manual paragraph 11.4](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#11.4%20Initialization).

```C
// 1. Initialize the pin functions:
//    -  Initialize single pin functions by writing appropriate values to
//       PCRn
//    -  Initialize multiple pins (up to 16) with the same configuration by
//       writing appropriate values to Global Pin Control Low (GPCLR) or
//       Global Pin Control High (GPCHR).
// 2. Lock the configuration for a given pin, by writing 1 to PCRn [LK], so
//    that it cannot be changed until the next reset.
```

There is a PCR register for each and every pin. The reference manual describes the PCR registers in detail.

`?` Find the PCR register description for pin 7 in the reference manual.

<details>
<summary>Solution</summary>
See sub-paragraph 11.6.1.13 Pin Control 7 (PCR7)
</details><br>

Each PORT contains up to 32 PCR registers. These registers all have the same layout, so they are organized in an array. The PCR register for P1_7 is addressed as follows:

```C
PORT1->PCR[7]
```

`?` Use this register and write a C instruction for pin P1_7 to:

- Set the LK bit field to [1]: Lock this PCR. *Use the define PORT_PCR_LK(1).*
- Set the IBE bit field to [1]: Input Buffer Enable. *Use the define PORT_PCR_IBE(1).*
- Set the MUX bit field to [0000]: alternative 0 (GPIO). *Use the define PORT_PCR_MUX(0).*
- Set all other bit fields to their reset value logic 0

<details>
<summary>Solution</summary>
```C
// 1. & 2.
//
// Configure pins
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
</details><br>

## Reading the pin logic state

After initialization the pin logic state can be read from [PDIR](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#12.7.1.8%20Port%20Data%20Output%20(PDIR)) register in the GPIO peripheral. For example:

```C
while(1)
{
    // SW3 pressed?
    if((GPIO1->PDIR & (1<<7)) == 0)
    {
        // Blue LED on
        GPIO3->PCOR = (1<<0);
    }
    else
    {
        // Blue LED off
        GPIO3->PSOR = (1<<0);
    }
}
```

## Test and verification

- Open the project general_purpose_input.
- Build and run the application.
- Verify that the blue LED is one when SW3 is pressed.

## Final assignment

Remove (or comment) the while-loop in main.c. Create a new while-loop and toggle the blue LED when SW3 is pressed.

Some tips and hints:

- Use register GPIO1->PDIR to get the current pin state.
- Use a variable to check if the the current pin state is different from the previous check.
- If the pin state is different from the previous check:
    - Save this state
    - Is SW3 pressed (in other words, does it read logic 0)?
        - Toggle the blue LED by using the GPIO3->PTOR register.

# General Purpose Output

## Goal

Make the RGB LEDs blink by using the GPIO module.

## Required hardware

- FRDM-MCXA153A board

## Which pins to use?

Before being able to control the RGB LEDs, the physical connection on the FRDM-MCXA153 board must be checked. Refer to the [board schematic page 7](./../../docs/datasheets/SPF-90829_A1.pdf#page=7).

`?` What microcontroller pins are connected to the RGB LED?

<details>
<summary>Solution</summary>
```C
// LED_RED:   PORT3 pin 12 (P3_12)
// LED_GREEN: PORT3 pin 13 (P3_13)
// LED_BLEU:  PORT3 pin 0 (P3_0)    
```
</details><br>

## Initialization

GPIO modules can be used to control the logic state of a microcontroller pin. However, a pin must first be configured for the GPIO function. Using P3_13 as an example, it takes the following steps to configure the pin for GPIO:

1. Enable the PORT3 and GPIO3 modules in the MRCC module.
2. Initialize P3_13 for GPIO function in the PORT3 module.
3. Initialize P3_13 for output function in the GPIO3 module.

### 1. Enable the PORT3 and GPIO3 modules

The MRCC module must be used to enable other modules, such as PORT3 and GPIO3. How this is done is described in the [reference manual paragraph 14.3](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#14.3%20Functional%20description).

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
// GPIO3: [1] = Peripheral clock is enabled
// PORT3: [1] = Peripheral clock is enabled
MRCC0->MRCC_GLB_CC1 |= (1<<8) | (1<<0);

// Release modules from reset and leave others unchanged
// GPIO3: [1] = Peripheral is released from reset
// PORT3: [1] = Peripheral is released from reset
MRCC0->MRCC_GLB_RST1 |= (1<<8) | (1<<0);
```

`?` Why are the bits 0 and 8 set? *Tip: refer to MRCC_GLB_CC1 and MRCC_GLB_RST register description in the reference manual.*

<details>
<summary>Solution</summary>
```C
// According the MRCC_GLB_CC1 and MRCC_GLB_RST register diagrams:
// - GPIO3 is bit position 8
// - PORT3 is bit position 0
```
</details><br>

### 2. Initialize P3_13 for GPIO function

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

`?` Find the PCR register description for pin 13 in the reference manual.

<details>
<summary>Solution</summary>
See sub-paragraph 11.6.1.16 Pin Control a (PCR10 - PCR13)
</details><br>

Each PORT contains up to 32 PCR registers. These registers all have the same layout, so they are organized in an array. The PCR register for P3_13 is addressed as follows:

```C
PORT3->PCR[13]
```

`?` Use this register and write a C instruction for pin P3_13 to:

- Set the LK bit field to [1]: Lock this PCR
- Set the MUX bit field to [0000]: alternative 0 (GPIO)
- Set all other bit fields to their reset value logic 0

<details>
<summary>Solution</summary>
```C
// 1. & 2.
//
// Configure P3_13
// LK : [1] = Locks this PCR
// INV: [0] = Does not invert
// IBE: [0] = Disables
// MUX: [0000] = Alternative 0 (GPIO)
// DSE: [0] = low drive strength is configured on the corresponding pin,
//            if the pin is configured as a digital output
// ODE: [0] = Disables
// SRE: [0] = Fast
// PE:  [0] = Disables
// PS:  [0] = n.a.
PORT3->PCR[13] = 0x00008000;
```
</details><br>

### 3. Initialize P3_13 for output function

Refer to the [reference manual paragraph 12.5](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#12.5%20Initialization). 

`?` What description is provided to initialize a pin for the output function? *Note: the interrupt function is not used.*

<details>
<summary>Solution</summary>
```C
// 1. Initialize the GPIO pins for the output function:
//    a. Configure the output logic value for each pin by using Port Data
//       Output (PDOR).
//    b. Configure the direction for each pin by using Port Data Direction
//       (PDDR).
// 2. Interrupt function not used.
```
</details><br>

This description tells that there is a PDOR and PDDR register for the GPIO pins. For P3_13, this registers are addressed as follows:

```C
GPIO3->PDOR
GPIO3->PDDR
```

Manipulating the 13th bit in these registers, manipulates pin P3_13. 

`?` What C instructions can thus be used to initialize P3_13 as an output pin? Use the description of the PDOR and PDDR registers in [reference manual paragraph 12.7](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#12.7%20Memory%20map%20and%20register%20definition). *Note: write the instruction such that the 13th bit is updated only!*

<details>
<summary>Solution</summary>
```C
// 1. a.
//
// PDO13: [1] = Logic level 1 – LED green off
GPIO3->PDOR |= (1<<13);

// 1. b.
//
// PDD13: [1] = Output
GPIO3->PDDR |= (1<<13);
```
</details><br>

## Pin toggling

After initialization the pin can be toggled by using the [PDOR](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#12.7.1.4%20Port%20Data%20Output%20(PDOR)) register in the GPIO peripheral. Setting a bit in the PDOR can also be done by using the [PSOR](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#12.7.1.4%20Port%20Set%20Output%20(PSOR)) register. For resetting a bit in the PDOR register, the [PCOR](./../../docs/datasheets/MCXAP64M96FS3RM.pdf#12.7.1.4%20Port%20Clear%20Output%20(PCOR)) register can be used.

`?` Finish the following while-loop:

```C
while(1)
{
    // LED green off
    // Write logic 1 to bit 13 in the GPIO3 PSOR register so the
    // corresponding bit in PDOR becomes 1
    < your code here >

    // Delay
    for(volatile int i=0; i<1000000; i++)
    {}

    // LED green on
    // Write logic 1 to bit 13 in the GPIO3 PCOR register so the
    // corresponding bit in PDOR becomes 0
    < your code here >

    // Delay
    for(volatile int i=0; i<1000000; i++)
    {}
}
```

<details>
<summary>Solution</summary>
```C
while(1)
{
    // LED green off
    // Write logic 1 to bit 13 in the GPIO3 PSOR register so the
    // corresponding bit in PDOR becomes 1
    GPIO3->PSOR = (1<<13);

    // Delay
    for(volatile int i=0; i<1000000; i++)
    {}

    // LED green on
    // Write logic 1 to bit 13 in the GPIO3 PCOR register so the
    // corresponding bit in PDOR becomes 0
    GPIO3->PCOR = (1<<13);

    // Delay
    for(volatile int i=0; i<1000000; i++)
    {}
}
```
</details><br>

The same blinking functionality could have been achieved by using the PDOR register of PORT3. For example, to turn off the green LED:
```C
GPIO3->PDOR |= (1<<13);
```

`?` Can you think of a reason why the registers PSOR and PCOR are available?

<details>
<summary>Answer</summary>
```C
// By using the PSOR and PCOR registers, there is no need to read the content of 
// the PDOR register for updating a single bit. If the bitwise operators must be 
// used, there are three actions involved: Read-Modify-Write. Hence, this is not
// not an atomic action, and can thus be interrupted. Writing to the PSOR or PCOR
// register is an atomic action.
```
</details><br>

## Test and verification

- Open the project general_purpose_output.
- Build and run the application.
- Verify that the green LED is blinking.

## Final assignment

- Update the while-loop to show the following blinking sequence:
```C
red -> green -> blue -> off -> red -> green -> blue -> off -> etc.
```
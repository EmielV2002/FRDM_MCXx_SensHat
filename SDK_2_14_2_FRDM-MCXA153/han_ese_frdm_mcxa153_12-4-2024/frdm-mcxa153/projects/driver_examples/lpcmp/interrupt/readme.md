Overview
========
This application demonstrates how to initialize and use the CMP module to compare the voltage on an input pin to the internal DAC voltage.
An interrupt is generated on rising edges.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Potentiometer (any value between 10k - 100k Ohm), connected to P2_2/ADC0_A4/CMP0_IN0, 3V3, and GND.

Board settings
==============
- Default

Preparation
===========
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Debug the application.

Result
======
- In main.c watch the variable *cnt*. Its value increments by one every time the potentiometer sweeps from below 1.65V to above 1.65V.

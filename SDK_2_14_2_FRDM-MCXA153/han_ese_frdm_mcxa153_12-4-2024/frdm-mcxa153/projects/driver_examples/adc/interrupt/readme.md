Overview
========
This application demonstrates how to initialize and use the ADC module to start a conversion and wait for it to complete by using an interrupt.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Potentiometer (any value between 10k - 100k Ohm), connected to P1_10/ADC0_A8, 3V3, and GND.

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
- In main.c watch the variable *result*. Its value changes between 0 and 65535 with the position of the potentiometer.

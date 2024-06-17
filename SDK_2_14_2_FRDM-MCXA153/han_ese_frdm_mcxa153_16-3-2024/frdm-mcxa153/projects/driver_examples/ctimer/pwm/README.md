Overview
========
This application demonstrates how to configure the CTimer module to create a PWM signal.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Logic analyzer

Board settings
==============
- Default

Preparation
===========
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.
- Connect a logic analyzer to pin P2_4.

Result
======
- The logic analyzer shows the generated PWM signal:
    - frequency = 1 kHz
    - duty cycle = 2/3

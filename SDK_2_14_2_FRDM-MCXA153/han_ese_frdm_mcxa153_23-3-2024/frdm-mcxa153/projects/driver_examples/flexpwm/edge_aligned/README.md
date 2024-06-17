Overview
========
This application demonstrates how to configure the FlexPWM module to create a 16-bit precision edge aligned PWM signal.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Logic analyzer (optional)

Board settings
==============
- Default

Preparation
===========
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.
- Optional: connect a logic analyzer to pin P3_13.

Result
======
- The brightness of the green RGB LED is rapidly increasing.
- The logic analyzer shows the generated PWM signal with a frequency of 366.21 Hz. 

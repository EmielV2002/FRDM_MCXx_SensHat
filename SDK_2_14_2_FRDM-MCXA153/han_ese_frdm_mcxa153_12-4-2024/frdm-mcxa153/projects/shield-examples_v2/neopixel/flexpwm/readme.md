Overview
========
This application demonstrates how to use the FLexPWM module to control the neopixels. Neopixels are controlled with a PWM signal that has two possible duty cycles. The duration of a duty cycle is determined by the compare value. These compare values are updated in an interrupt handler as soon as the FlexPWM submodule counter overflows.

For this demonstration, the core clock is running at 96 MHz instead of the default 48 MHz. The reason for this, is that the period time of the PWM signal is 1 microsecond. So within this microsecond, the new compare value must be written in the VAL3 register by the interrupt handler. At 48 MHz, testing showed that this could not be achieved.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Shield

Board settings
==============
- Default

Preparation
===========
- Connect the RGB Clock shield to the FRDM-MCXA153 board.
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

Result
======
- Each RGB LED lights show a chaser going clockwise. After a finished round, the colour changes from red, to green, to blue.
- The direction of the chaser can be changed by pressing SW3 or 'd' via a terminal application.

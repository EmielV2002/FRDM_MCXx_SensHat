Overview
========
This application demonstrates how to use the GPIO module in output mode and polling to control the twelve neopixels on the RGB Clock shield.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- RGB Clock shield

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
- The RGB LEDs show a chaser going clockwise. After a finished round, the colour changes from red, to green, to blue.
- If the first RGB LED is on, a beeping sound is generated. The frequency of the beeping sound changes with the colour.
- The direction of the chaser changes by pressing SW3.
- If the chaser's direction is counter clockwise, no beep is generated.

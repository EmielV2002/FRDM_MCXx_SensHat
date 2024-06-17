Overview
========
This application demonstrates how to use the GPIO module in input mode to detect rotary encoder movement.
One input pin is configured for generating interrupts and in the ISR the other pin is sampled.

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
- Connect the shield to the FRDM-MCXA153 board.
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Debug the application.

Result
======
- Observe the value of the variable cnt in a watch window. It will increment on clockwise rotation and decrement on counter clockwise rotation


Overview
========
This application demonstrates how to use the LPSPI module to communicate with the M950x0-R EEPROM.

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
- Connect the shield to the FRDM-MCXA153 board.
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Debug the application.

Result
======
- The application has two modes, which are selected by a define

    - #if 1: This is a read example. Use the debugger to visualize the received data.
    - #if 0: This is a write example. After using the write example, switch back to the read example and see if the data has been updated.

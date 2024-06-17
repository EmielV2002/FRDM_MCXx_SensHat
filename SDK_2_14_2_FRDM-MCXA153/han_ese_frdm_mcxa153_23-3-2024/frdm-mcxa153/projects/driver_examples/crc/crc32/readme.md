Overview
========
This application demonstrates how to initialize and use the CRC module to calculate a CRC-32 checksum.

Details
====================
Do you want to know more details about this example? Check the [details](./readme_details.md).

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable

Board settings
==============
- Default

Preparation
===========
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

Result
======
- The green RGB LED is blinking, indicating all CRC-32 calculations are correct.
- By making a change to any of the data values, the red RGB LED starts blinking, indicating a CRC-32 calculations is incorrect.

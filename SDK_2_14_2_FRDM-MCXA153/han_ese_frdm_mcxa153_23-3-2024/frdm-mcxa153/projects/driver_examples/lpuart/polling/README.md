Overview
========
This application demonstrates how to initialize and use the LPUART module to transmit and receive data by using polling.

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
- Open the VCOM (9600-8n1) in a terminal application of your choice.
- Verify that after microcontroller reset a message appears in the terminal application.
- Verify that typing the characters 'r', 'g' and 'b' in the terminal application toggles the corresponding RBG LED.

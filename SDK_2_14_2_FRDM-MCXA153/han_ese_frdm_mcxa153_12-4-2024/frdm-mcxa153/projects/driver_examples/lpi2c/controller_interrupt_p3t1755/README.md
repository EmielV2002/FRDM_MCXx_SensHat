Overview
========
This application demonstrates how to configure and use the LPI2C module in controller mode by using interrupts for transmitting and receiving data. It reads the temperature from the P3T1755 digital temperature sensor that is available on the FRDM-MCXA153 board.

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
- In the debugger, watch the variable **temperature**. The value reflects the ambient temperature.

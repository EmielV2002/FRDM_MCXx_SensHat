Overview
========
This application demonstrates what happens if critical sections are not properly protected from pre-emption. The application prints a line in a terminal application and should display the same number between 0 and 9 on one line. However, the critical section (copying the global buffer in main to a local buffer before printing) is not protected from pre-emption. This means that if the timing is right, the printed line might show different numbers.

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
- Press SW3. Try to get the timing right and trigger the race condition (the buffer printing different values on a single SW3 click).

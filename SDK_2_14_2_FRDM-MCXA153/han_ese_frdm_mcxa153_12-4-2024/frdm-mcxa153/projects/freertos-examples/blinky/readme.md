Overview
========
This application demonstrates how to configure FreeRTOS. It demonstrates:

- How to create two tasks.
- How to implement thread safe interrupt driven LPUART communication by using a queue.
- How to generate runtime statistics that can be visualized by a kernel aware debugger.

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
- A message is printed after a microcontroller reset.
- Every 250 ms: task 1 prints a message and blinks the green RGB LED.
- Every 1000 ms: task 2 prints a message.


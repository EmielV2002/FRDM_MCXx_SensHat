Overview
========
This application demonstrates how to configure and use the LPSPI modules in both master and slave mode by using interrupts for transmitting and receiving data. The demonstration takes advantage of the fact that there are two LPSPI modules available in the MCX153VLH microcontroller. Data is transmitted from master to slave and from slave to master by using software FIFOs.

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
- Connect the four jumper wires as follows:

    | Master           | Slave            |
    |------------------|------------------|
    | P1_0/LPSPI0_SDO  | P2_16/LPSPI1_SDI |
    | P1_1/LPSPI0_SCK  | P2_12/LPSPI1_SCK |
    | P1_2/LPSPI0_SDI  | P2_13/LPSPI1_SDO |
    | P1_3/LPSPI0_PCS0 | P2_6/LPSPI1_PCS1 |

- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

Result
======
- In the debugger, check to see if the application executes one of the while(1) endless loops. If so, a transmission error has occurred.

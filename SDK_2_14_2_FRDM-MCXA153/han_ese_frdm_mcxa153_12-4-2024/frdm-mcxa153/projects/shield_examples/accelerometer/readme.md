Overview
========
This application demonstrates how to use the LPSPI module to communicate with the FXLS8974CF 3-Axis Low-g Accelerometer.

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
- The application:

    - reads and verifies the WHO_AM_I register
    - enables the ACTIVE bit in the SENS_CONFIG1 register
    - reads the TEMP_OUT register and calculates the temperature

- Use a watch window to see the received data and the temperature. Notice that the temperature has a sensitivity of 1 degree Celsius per LSB.

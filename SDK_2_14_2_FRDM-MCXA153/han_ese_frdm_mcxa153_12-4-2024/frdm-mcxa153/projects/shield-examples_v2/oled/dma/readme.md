Overview
========
This application demonstrates how to configure and use the LPI2C module in controller mode by using the DMA module for transmitting data to an SSD1306 OLED display.

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- 128x64 SSD1306 OLED display
- Jumper wires

Board settings
==============
- Default

Preparation
===========
- Connect jumper wires as follows:

    | FRDM-MCXA153    | OLED |
    |-----------------|------|
    | P1_8/LPI2C0_SDA | SDA  |
    | P1_9/LPI2C0_SCL | SCL  |
    | 3V3             | VCC  |
    | GND             | GND  |

- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

Result
======
- The OLED display alternately shows an NXP and HAN logo and texts.

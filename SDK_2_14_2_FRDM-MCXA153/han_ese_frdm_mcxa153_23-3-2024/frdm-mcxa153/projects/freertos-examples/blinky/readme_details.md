# Getting Started

## Hardware

The NXP FRDM-MCXA153 development board is used in this course.
[![FRDM-MCXA153](https://www.nxp.com/assets/images/en/dev-board-image/FRDM-MCXA153-TOP.jpg)](https://www.nxp.com/design/design-center/development-boards/general-purpose-mcus/frdm-development-board-for-mcx-a14x-a15x-mcus:FRDM-MCXA153)

A board overview, product details, documentation and other resources is provided on the [NXP website](https://www.nxp.com/design/design-center/development-boards/general-purpose-mcus/frdm-development-board-for-mcx-a14x-a15x-mcus:FRDM-MCXA153). A board can purchased from ARLE, the NXP website or other electronics suppliers.

## Software

The preferred SDE is Keil MDK-ARM. This SDE is downloaded from the ARM Keil website:

[Keil MDK-ARM (MDK-Arm)](https://www.keil.com/download/product/)

1. Select the product called MDK-Arm.
2. Registration is required for the free version of this product.
3. Once installed, projects are opened by double clicking the **UVPROJX file**. There is such a file for each project and the file is located in the **mdkarm folder** in the project.

For your convenience, project files are also provided for other IDEs.

## Download SDK files

The provided projects will *not* compile without the SDK files provided by NXP. The SDK files can be downloaded by using the [SDK builder for the FRDM-MCXA153](https://mcuxpresso.nxp.com/en/builder?hw=FRDM-MCXA153) from the NXP website.

Select the following options:

- Host OS: your operating system
- Toolchain / IDE: All Toolchains
- Select all options:
    - CMSIS DSP Library
    - FreeMASTER
    - Motor Control Software
    - USB Host, Device, OTG Stack
    - FreeRTOS

The downloaded/cloned ***han_ese_frdm_mcxa153*** folder ***MUST*** be located in the root directory of the downloaded (and unzipped) SDK folder.

```txt
├─SDK_x_y_z_FRDM-MCXA153
  ├─boards
  ├─CMSIS
  ├─components
  ├─devices
  ├─docs
  ├─han_ese_frdm_mcxa153   <--  han_ese_frdm_mcxa153 FOLDER GOES HERE
  ├─middleware
  ├─rtos
  ├─tools
  ├─COPYING-BSD-3
  ├─FRDM-MCXA153_manifest_v3_13.xml
  ├─LA_OPT_NXP_Software_License.txt
  └─SW-Content-Register.txt
```

SDK versions that have been tested successfully:

- 2.14.2

## Update MCU-Link firmware

The FRDM-MCXA153 board comes with an [MCU-Link debug probe](https://docs.nxp.com/bundle/UM12012/page/topics/MCU_Link_OB_debug_probe.html). Out of the box, the CMSIS-DAP firmware is programmed. However, J-Link firmware is preferred. Changing the MCU-Link to J-Link firmware is described [here](https://docs.nxp.com/bundle/UM12012/page/topics/Updating_MCU_Link_firmware.html).

## Run the application

Run the application as described in the [readme](./readme.md)

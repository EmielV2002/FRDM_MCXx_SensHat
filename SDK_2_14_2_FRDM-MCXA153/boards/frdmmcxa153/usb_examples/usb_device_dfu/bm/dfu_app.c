/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "dfu_app.h"

#include "dfu.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#include "dfu_timer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
static void USB_DeviceApplicationInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* DFU demo configuration */
extern usb_device_class_struct_t g_UsbDeviceDfuDemoConfig;
extern uint8_t g_detachRequest;

/* Instant of DFU  structure */
usb_device_dfu_app_struct_t g_UsbDeviceDfu;

/* DFU configuration */
usb_device_class_config_struct_t g_UsbDeviceDfuConfig[USB_DFU_INTERFACE_COUNT] = {{
    USB_DeviceDfuDemoCallback, /* DFU demo class callback pointer */
    (class_handle_t)NULL,      /* The DFU class handle, This field is set by USB_DeviceClassInit */
    &g_UsbDeviceDfuDemoConfig, /* The DFU demo configuration, including class code, subcode, and protocol, class
                                  type */
}};
/* DFU configuration list */
usb_device_class_config_list_struct_t g_UsbDeviceDfuConfigList = {
    g_UsbDeviceDfuConfig,    /* Class configurations */
    USB_DeviceCallback,      /* Device callback pointer */
    USB_DFU_INTERFACE_COUNT, /* Class count */
};
/*******************************************************************************
 * Code
 ******************************************************************************/

void SysTick_Handler(void)
{
    DFU_TimerISR();
}

void DFU_TimerHWInit()
{
    /* Configure the SysTick timer */
    SysTick_Config(SystemCoreClock / 1000U);
    /* Disable the SysTick timer */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void HW_TimerControl(uint8_t enable)
{
    if (enable)
    {
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    }
    else
    {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    }
}

#if (defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceKhciIsrFunction(g_UsbDeviceDfu.deviceHandle);
}
#endif

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    RESET_PeripheralReset(kUSB0_RST_SHIFT_RSTn);
    CLOCK_EnableUsbfsClock();
#endif
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    uint8_t usbDeviceKhciIrq[] = USBFS_IRQS;
    irqNumber                  = usbDeviceKhciIrq[CONTROLLER_ID - kUSB_ControllerKhci0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    USB_DeviceKhciTaskFunction(deviceHandle);
#endif
}
#endif
/*!
 * @brief USB device callback function.
 *
 * This function handles the USB device specific requests.
 *
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            /* USB bus reset signal detected */
            g_UsbDeviceDfu.attach               = 0U;
            g_UsbDeviceDfu.currentConfiguration = 0U;
            error                               = kStatus_USB_Success;
            USB_DeviceDfuBusReset();
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_UsbDeviceDfu.speed))
            {
                USB_DeviceSetSpeed(handle, g_UsbDeviceDfu.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_UsbDeviceDfu.attach               = 0U;
                g_UsbDeviceDfu.currentConfiguration = 0U;
                error                               = kStatus_USB_Success;
            }
            else if (USB_DFU_CONFIGURE_INDEX == (*temp8))
            {
                /* Set device configuration request */
                g_UsbDeviceDfu.attach               = 1U;
                g_UsbDeviceDfu.currentConfiguration = *temp8;
                error                               = kStatus_USB_Success;
            }
            else
            {
                /* no action required, the default return value is kStatus_USB_InvalidRequest. */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (0U != g_UsbDeviceDfu.attach)
            {
                /* Set device interface request */
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface < USB_DFU_INTERFACE_COUNT)
                {
                    if (alternateSetting < USB_DFU_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_UsbDeviceDfu.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                      = kStatus_USB_Success;
                    }
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (NULL != param)
            {
                /* Get current configuration request */
                *temp8 = g_UsbDeviceDfu.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (NULL != param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_DFU_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_UsbDeviceDfu.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (NULL != param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (NULL != param)
            {
                /* Get device configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (NULL != param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventVendorRequest:
        {
            error = USB_DeviceGetVerdorDescriptor(handle, param);
        }
        break;
        default:
            /* no action required, the default return value is kStatus_USB_InvalidRequest. */
            break;
    }
    return error;
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0U);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* Set dfu device to default state */
    g_UsbDeviceDfu.speed  = USB_SPEED_FULL;
    g_UsbDeviceDfu.attach = 0U;

    g_UsbDeviceDfu.deviceHandle = NULL;
    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceDfuConfigList, &g_UsbDeviceDfu.deviceHandle))
    {
        usb_echo("USB device dfu demo init failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device dfu demo\r\n");

        USB_DeviceDfuDemoInit();
    }

    /* Install isr, set priority, and enable IRQ. */
    USB_DeviceIsrEnable();

    /* Start the device function */
    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_UsbDeviceDfu.deviceHandle);
}
/*!
 * @brief Main function.
 *
 * This function serves as the starting point for program execution.
 *
 * @return None.
 */
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    /* attach FRO 12M to LPUART0 (debug console) */
    RESET_PeripheralReset(kLPUART0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kPORT0_RST_SHIFT_RSTn);

    CLOCK_SetClockDiv(kCLOCK_DivLPUART0, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    USB_DeviceApplicationInit();
    while (1U)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_UsbDeviceDfu.deviceHandle);
#endif
        USB_DeviceDfuTask();
    }
}

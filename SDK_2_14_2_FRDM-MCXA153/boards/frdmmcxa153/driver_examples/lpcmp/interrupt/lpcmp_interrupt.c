/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_lpcmp.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPCMP_BASE             CMP0
#define DEMO_LPCMP_USER_CHANNEL     0U
#define DEMO_LPCMP_DAC_CHANNEL      7U
#define DEMO_LPCMP_IRQ_ID           CMP0_IRQn
#define LED_INIT()                  LED_RED_INIT(LOGIC_LED_OFF)
#define LED_ON()                    LED_RED_ON()
#define LED_OFF()                   LED_RED_OFF()
#define DEMO_LPCMP_IRQ_HANDLER_FUNC CMP0_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    lpcmp_config_t mLpcmpConfigStruct;
    lpcmp_dac_config_t mLpcmpDacConfigStruct;

    /* Initialize hardware. */
    /* attach peripheral clock */
    CLOCK_AttachClk(kFRO12M_to_CMP0);
    CLOCK_SetClockDiv(kCLOCK_DivCMP0_FUNC, 1U);

    /* enable CMP0 and CMP0_DAC */
    SPC0->ACTIVE_CFG1 |= ((1U << 16U) | (1U << 20U));

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("LPCMP Interrupt Example.\r\n");

    /*
     *   k_LpcmpConfigStruct->enableStopMode      = false;
     *   k_LpcmpConfigStruct->enableOutputPin     = false;
     *   k_LpcmpConfigStruct->useUnfilteredOutput = false;
     *   k_LpcmpConfigStruct->enableInvertOutput  = false;
     *   k_LpcmpConfigStruct->hysteresisMode      = kLPCMP_HysteresisLevel0;
     *   k_LpcmpConfigStruct->powerMode           = kLPCMP_LowSpeedPowerMode;
     *   k_LpcmpConfigStruct->functionalSourceClock = kLPCMP_FunctionalClockSource0;
     */
    LPCMP_GetDefaultConfig(&mLpcmpConfigStruct);
    /* Init the LPCMP module. */
    LPCMP_Init(DEMO_LPCMP_BASE, &mLpcmpConfigStruct);

    /* Configure the internal DAC to output half of reference voltage. */
    mLpcmpDacConfigStruct.enableLowPowerMode     = false;
    mLpcmpDacConfigStruct.referenceVoltageSource = kLPCMP_VrefSourceVin2;
    mLpcmpDacConfigStruct.DACValue =
        ((LPCMP_DCR_DAC_DATA_MASK >> LPCMP_DCR_DAC_DATA_SHIFT) >> 1U); /* Half of reference voltage. */
    LPCMP_SetDACConfig(DEMO_LPCMP_BASE, &mLpcmpDacConfigStruct);

    /* Configure LPCMP input channels. */
    LPCMP_SetInputChannels(DEMO_LPCMP_BASE, DEMO_LPCMP_USER_CHANNEL, DEMO_LPCMP_DAC_CHANNEL);

    /* Init the LED. */
    LED_INIT();

    /* Enable the interrupt. */
    EnableIRQ(DEMO_LPCMP_IRQ_ID);
    LPCMP_EnableInterrupts(DEMO_LPCMP_BASE, kLPCMP_OutputRisingInterruptEnable | kLPCMP_OutputFallingInterruptEnable);

    while (1)
    {
    }
}

/*!
 * @brief ISR for LPCMP interrupt function.
 */
void DEMO_LPCMP_IRQ_HANDLER_FUNC(void)
{
    LPCMP_ClearStatusFlags(DEMO_LPCMP_BASE, kLPCMP_OutputRisingEventFlag | kLPCMP_OutputFallingEventFlag);
    if (kLPCMP_OutputAssertEventFlag == (kLPCMP_OutputAssertEventFlag & LPCMP_GetStatusFlags(DEMO_LPCMP_BASE)))
    {
        LED_ON(); /* Turn on the led. */
    }
    else
    {
        LED_OFF(); /* Turn off the led. */
    }
    SDK_ISR_EXIT_BARRIER;
}

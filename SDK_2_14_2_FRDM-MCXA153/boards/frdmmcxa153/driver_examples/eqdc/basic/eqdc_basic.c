/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_eqdc.h"
#include "fsl_debug_console.h"

#include "fsl_inputmux.h"
#include "fsl_inputmux_connections.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_EQDC QDC0
#define DEMO_INPUTMUX INPUTMUX0

#define DEMO_ENCODER_DISK_LINE 0xFFFFFFFFU
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    eqdc_config_t sEqdcConfig;
    uint32_t mCurPosValue;
    
    RESET_PeripheralReset(kQDC0_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    INPUTMUX_Init(DEMO_INPUTMUX);
    INPUTMUX_AttachSignal(DEMO_INPUTMUX, 0, kINPUTMUX_TrigIn10ToQdc0Phasea);
    INPUTMUX_AttachSignal(DEMO_INPUTMUX, 0, kINPUTMUX_TrigIn5ToQdc0Phaseb);
    INPUTMUX_AttachSignal(DEMO_INPUTMUX, 0, kINPUTMUX_TrigIn2ToQdc0Index);
    
    PRINTF("\r\nEQDC Basic Example.\r\n");

    EQDC_GetDefaultConfig(&sEqdcConfig);
    sEqdcConfig.positionModulusValue               = DEMO_ENCODER_DISK_LINE;
    
    EQDC_Init(DEMO_EQDC, &sEqdcConfig);
    EQDC_SetOperateMode(DEMO_EQDC, kEQDC_QuadratureDecodeOperationMode);
    EQDC_DoSoftwareLoadInitialPositionValue(DEMO_EQDC);

    PRINTF("Press any key to get the encoder values ...\r\n");

    while (1)
    {
        GETCHAR();
        PRINTF("\r\n");

        /* This read operation would capture all the position counter to responding hold registers. */
        mCurPosValue = EQDC_GetPosition(DEMO_EQDC);
        
        PRINTF("Current position value: %ld\r\n", mCurPosValue);
        PRINTF("Position differential value: %d\r\n", (int16_t)EQDC_GetHoldPositionDifference(DEMO_EQDC));
        PRINTF("Position revolution value: %d\r\n", EQDC_GetHoldRevolution(DEMO_EQDC));
        PRINTF("\r\n");
    }
}

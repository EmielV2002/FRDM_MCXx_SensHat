/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_
#define _POWER_MODE_SWITCH_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _app_power_mode
{
    kAPP_PowerModeMin = 'A' - 1,
    kAPP_PowerModeActive,        /* Normal RUN mode. */
    kAPP_PowerModeSleep,         /* Sleep. */
    kAPP_PowerModeDeepSleep,     /* DeepSleep */
    kAPP_PowerModePowerDown,     /* PowerDown */
    kAPP_PowerModeDeepPowerDown, /* DeepPowerDown. */
    kAPP_PowerModeMax
} app_power_mode_t;

#endif /*_POWER_MODE_SWITCH_*/

/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MID_SM_STATES_H_
#define _MID_SM_STATES_H_

#include "mid_def.h"
#include "mcaa_lib.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern middef_pmsm_t g_sMidDrive;
extern mid_struct_t g_sMID;
extern MCAA_ESTIMRL_T_F32 g_sEstimRLStruct;
extern MCAA_ESTIMRL_RUN_T_F32 g_sEstimRLCtrlRun;
extern rl_estim_cfg_params_t g_sEstimRLInitFMSTR;
extern volatile float g_fltMIDDCBvoltageScale;
extern volatile float g_fltMIDcurrentScale;
extern volatile float g_fltMIDparamScale;
extern volatile float g_fltMIDspeedScale;
extern frac32_t f32IDcPlot;
extern frac32_t f32LdPlot;
extern frac32_t f32LqPlot;

/*******************************************************************************
 * API
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief  MID fast-loop process function.
 *
 * @param  void
 *
 * @return none
 */
void MID_ProcessFast_FL(void);

/*!
 * @brief  MID initialization function.
 *
 * @param  void
 *
 * @return none
 */
void MID_Init(void);

/*!
 * @brief   Function returns actual MID state.
 *
 * @param   None
 *
 * @return  Actual MID state
 */
uint16_t MID_GetActualState(void);

#endif /* _MID_SM_STATES_ */

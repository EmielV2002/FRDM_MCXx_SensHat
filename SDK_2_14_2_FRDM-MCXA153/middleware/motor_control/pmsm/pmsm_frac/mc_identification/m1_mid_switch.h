/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _M1_MID_SWITCH_H_
#define _M1_MID_SWITCH_H_

#ifdef PMSM_SNSLESS
#include "m1_sm_snsless.h"
#define MC_TEMP 1
#endif

#ifdef MID_EN
#include "mid_sm_states.h"
#define MID_TEMP 1
#endif

#if (MC_TEMP == 1 && MID_TEMP == 1)
#define M1_MID_SWITCH_SUPPORT
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* M1/MID switch request enumeration. */
typedef enum _m1_mid_request_t{
    kNoRequest  = 0U,
    kM1toMID    = 1U,
    kMIDtoM1    = 2U,
} m1_mid_request_t;

/* M1/MID application state enumeration. */
typedef enum _m1_mid_state_t{
    kStateM1     = 0U,
    kStateMID    = 1U,
} m1_mid_state_t;

/* M1/MID switch fault enumeration. */
typedef enum _m1_mid_fault_t{
    kNoFault         = 0U,
    kM1toMIDFault    = 1U,
    kMIDtoM1Fault    = 2U,
} m1_mid_fault_t;

/* M/MID switching structure. */
typedef struct _m1_mid_struct_t
{
    m1_mid_request_t eRequest;
    m1_mid_state_t   eAppState;
    m1_mid_fault_t   eFaultSwicth;
} m1_mid_struct_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern m1_mid_struct_t g_sM1toMID;

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
* @brief  Check M1/MID switch request.
*
* @param  void
*
* @return none
*/
void M1_MID_Switch_BL(void);

/*!
* @brief  Get actual application state (M1 or MID).
*
* @param  void
*
* @return Actual application state
*/
uint16_t M1_MID_Get_State(void);

#ifdef __cplusplus
}
#endif

#endif /* M1_MID_SWITCH */

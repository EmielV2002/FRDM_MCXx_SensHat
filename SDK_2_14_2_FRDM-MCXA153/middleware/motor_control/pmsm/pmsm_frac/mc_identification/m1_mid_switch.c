/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "m1_mid_switch.h"
   
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
m1_mid_struct_t g_sM1toMID;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
   
/*!
 * MID fast-loop process function.
 */
void M1_MID_Switch_BL(void)
{
#ifdef M1_MID_SWITCH_SUPPORT
  switch (g_sM1toMID.eAppState)
  {
  case kStateM1:
    /* Spin state machine is running. */
    if(g_sM1toMID.eRequest == kM1toMID)
    {
      if(M1_GetAppState() == kSM_AppStop)
      {
        MID_Init();                             /* Call MID init routine. */
        g_sM1toMID.eFaultSwicth = kNoFault;     /* Clear fault flag. */
        g_sM1toMID.eAppState = kStateMID;       /* Switch application state to MID */
      }
      else
      {
        g_sM1toMID.eFaultSwicth = kM1toMIDFault; /* M1 is not in STOP state. */
      }    
    }    
    g_sM1toMID.eRequest = kNoRequest;            /* Always clear request. */
    break;
    
  case kStateMID:
    /* MID state machine is running. */
    if(g_sM1toMID.eRequest == kMIDtoM1)
    {
      if(MID_GetActualState() == kMID_Stop)
      {
        g_sM1Ctrl.eState = kSM_AppInit;         /* Set Init state for M1 state machine */
        g_sM1toMID.eFaultSwicth = kNoFault;     /* Clear fault flag. */
        g_sM1toMID.eAppState = kStateM1;       /* Switch application state to M1 */
      }
      else
      {
        g_sM1toMID.eFaultSwicth = kMIDtoM1Fault; /* MID is not in STOP state. */
      }  
    } 
    g_sM1toMID.eRequest = kNoRequest;            /* Always clear request. */
    break;
    
  default:
    ;
  }
#endif
}

/*!
 * Function returns actual state of the application.
 */
uint16_t M1_MID_Get_State(void)
{
  return ((uint16_t)g_sM1toMID.eAppState);
}

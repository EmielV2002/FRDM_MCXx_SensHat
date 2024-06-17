/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mid_sm_states.h"
#include "mc_periph_init.h"
#include "pp_measure.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* (user) Motor parameters identification state machine functions */
static void MID_StateStart(void);
static void MID_StateRL(void);
static void MID_StatePp(void);
static void MID_StateMechParam(void);
static void MID_StateStop(void);
static void MID_StateFault(void);

/* (user) Motor parameters identification state-transition functions */
static void MID_TransStart2RL(void);
static void MID_TransStart2MechParam(void);
static void MID_TransStart2Pp(void);
static void MID_TransAll2Stop(void);
static void MID_TransAll2Fault(void);
static void MID_TransStop2Start(void);

static void MID_FaultDetection(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* State machine functions field */
const mid_pfcn_void_pms g_MID_SM_STATE_TABLE[6U] = {MID_StatePp, MID_StateRL, MID_StateMechParam, MID_StateStart, MID_StateStop, MID_StateFault};

/* Global structure for the motor FOC */
middef_pmsm_t g_sMidDrive;

/* Global structure for MID SM control etc. */
mid_struct_t g_sMID;

/* FreeMASTER scales - DO NOT USE THEM in the code to avoid float library include */
volatile float g_fltMIDDCBvoltageScale;
volatile float g_fltMIDcurrentScale;
volatile float g_fltMIDparamScale;
volatile float g_fltMIDspeedScale;

/* Variables for EstimRL */
MCAA_ESTIMRL_RUN_T_F32 g_sEstimRLCtrlRun;                     /* EstimRL - Control manual mode and measured values in modes 1, 2. */
MCAA_ESTIMRL_T_F32 g_sEstimRLStruct;                          /* EstimRL inner variables and signals */
frac32_t f32LdqTable[3*NUM_MEAS];                               /* Array for measuring DC current and estimated inductances. */
uint8_t u8PlotCnt;                                              /* Plot counter. */
frac32_t f32IDcPlot;                                            /* DC current [A]. */ 
frac32_t f32LdPlot;                                             /* Estimated d-axis inductance. */        
frac32_t f32LqPlot;                                             /* Estimated q-axis inductance. */
rl_estim_cfg_params_t g_sEstimRLInitFMSTR = {
    SCALE(I_NOMINAL), SCALE(I_POSMAX), SCALE(I_NEGMAX), SCALE(I_LD), SCALE(I_LQ), 0U};     /* Control structure used in FreeMASTER */


/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief MID START state
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_StateStart(void)
{
  /* Type the code to do when in the START state */

  if (--g_sMidDrive.ui16CounterState == 0U)
  {
    /* write calibrated offset values when calibration is done */
    M1_MCDRV_CURR_3PH_CALIB_SET(&g_sM1AdcSensor);
    
    /* Clear captured faults in MID */
    MID_FAULT_CLEAR_ALL(g_sMID.ui16FaultMID);

    switch (g_sMID.eMeasurementType)
      {
      case kMID_PolePairs:
        MID_TransStart2Pp();
        break;
        
      case kMID_ElectricalParams:
        MID_TransStart2RL();
        break;
        
      case kMID_MechanicalParams:
        MID_TransStart2MechParam();
        break;
        
      default:
        MID_TransAll2Stop();
      }
    
    /* Force sector to 4 to ensure that currents Ia, Ib will be sensed and Ic calculated */
    g_sMidDrive.sFocPMSM.ui16SectorSVM = 4;

  }
  else
  {
    /* call offset measurement */
    M1_MCDRV_CURR_3PH_CALIB(&g_sM1AdcSensor);

    /* change SVM sector in range <1;6> to measure all AD channel mapping combinations */
    if (++g_sMidDrive.sFocPMSM.ui16SectorSVM > 6U)
    {
        g_sMidDrive.sFocPMSM.ui16SectorSVM = 1U;
    }
  }
}

/*!
 * @brief MID PP state
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_StatePp(void)
{
    /* Type the code to do when in the PP state */

    /* Call Pp measurement routine */
    MID_getPp(&(g_sMID.sMIDPp));
    
    /* Perform Current control if MID_PP */
    g_sMidDrive.sFocPMSM.bCurrentLoopOn = TRUE;
    MCS_PMSMFocCtrl(&g_sMidDrive.sFocPMSM);
}

/*!
 * @brief MID RL state
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_StateRL(void)
{
    MCAA_ESTIMRL_RET_T eEstimRetVal;                      /* EstimRL return value */
    
    /* Type the code to do when in the RL state */

    /* Transform currents A,B,C to Alpha, Beta */
    GMCLIB_Clark_F16(&g_sMidDrive.sFocPMSM.sIABC, &g_sMidDrive.sFocPMSM.sIAlBe);

    eEstimRetVal = MCAA_EstimRL_F16(g_sMidDrive.sFocPMSM.f16UDcBus, &g_sMidDrive.sFocPMSM.sIAlBe, 
                                    &g_sEstimRLStruct, g_sEstimRLCtrlRun, &g_sMidDrive.sFocPMSM.sUAlBeReq);

    /* DCBus ripple elimination */
    GMCLIB_ElimDcBusRipFOC_F16(g_sMidDrive.sFocPMSM.f16UDcBus, &g_sMidDrive.sFocPMSM.sUAlBeReq, &g_sMidDrive.sFocPMSM.sUAlBeComp);
    
    /* Space vector modulation */
    g_sMidDrive.sFocPMSM.ui16SectorSVM = GMCLIB_SvmStd_F16(&g_sMidDrive.sFocPMSM.sUAlBeComp, &g_sMidDrive.sFocPMSM.sDutyABC);
    
    if(eEstimRetVal == RET_DONE)
    {
      /* Estimation successfully done, store the results */
      g_sMID.f32Rs = g_sEstimRLStruct.f32R;
      g_sMID.f32Ld = g_sEstimRLStruct.f32Ld;
      g_sMID.f32Lq = g_sEstimRLStruct.f32Lq;
    
      MID_TransAll2Stop();
    }
    else if(eEstimRetVal == RET_ERROR)
    {
      /* Fault occurs during EstimRL */
      g_sMID.ui16FaultMID |= MID_ESTIMRL_RUN_ERROR;
      MID_TransAll2Fault();
    }
}

/*!
 * @brief MID MECHPARAM state
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_StateMechParam(void)
{
    /* Type the code to do when in the MechParam state */

    MID_TransAll2Stop();
}

/*!
 * @brief MID STOP state
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_StateStop(void)
{
  /* Type the code to do when in the STOP state */

  /* Plot dq-inductance */
  f32IDcPlot = f32LdqTable[u8PlotCnt*3]; 
  f32LdPlot  = f32LdqTable[u8PlotCnt*3 + 1];
  f32LqPlot  = f32LdqTable[u8PlotCnt*3 + 2];

  u8PlotCnt++;
  if(u8PlotCnt>=NUM_MEAS)
          u8PlotCnt=0U;

  /* Wait in STOP unless measurement is triggered. */
  if(g_sMID.bMIDOnOff == 1)
  {     
    /* Go to MID START (ADC calibration) */
    MID_TransStop2Start();
  }
}

/*!
 * @brief MID FAULT state
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_StateFault(void)
{
    /* Type the code to do when in the FAULT state */
  
    /* After manual clear fault go to STOP state */
    if (!MID_FAULT_ANY(g_sMidDrive.sFaultIdCaptured))
    {
      MID_TransAll2Stop();
    }
}

/*!
 * @brief MID START to RL transition
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_TransStart2RL(void)
{
    static MCAA_ESTIMRL_INIT_T_F32 g_sEstimRLInitCfg = {SCALE(I_NOMINAL), 0U, 0, 0, 0};
    static MCAA_ESTIMRLINIT_RET_T eEstimRetValInit;              /* EstimRL initialization return value */

    /* Type the code to do when going from the Start to the RL state */   
    
    /* Identification init */    
    switch(g_sEstimRLInitFMSTR.u8ModeEstimRL)
    {
      case 1:
          /* Mode 1 */ 
          g_sEstimRLInitCfg.f32IDcMax = g_sEstimRLInitFMSTR.f32IDcPosMax;
          g_sEstimRLInitCfg.f32IDcLd = 0;
          g_sEstimRLInitCfg.f32IDcLq = g_sEstimRLInitFMSTR.f32IDcLq;
          g_sEstimRLInitCfg.f32IDcNegMax = 0;
          g_sEstimRLInitCfg.u16LdqNumMeas = NUM_MEAS;
          
          g_sEstimRLCtrlRun.pLdqTable = f32LdqTable;
          g_sEstimRLCtrlRun.f32IDcDReq = 0;
          g_sEstimRLCtrlRun.f32IDcQReq = 0;
          g_sEstimRLCtrlRun.f32IAcReq = 0;
          g_sEstimRLCtrlRun.u16FAc = 0U;               
          g_sEstimRLCtrlRun.u8LdqSwitch = 0U;      
          break;
      case 2:
          /* Mode 2 */
          g_sEstimRLInitCfg.f32IDcMax = g_sEstimRLInitFMSTR.f32IDcPosMax;
          g_sEstimRLInitCfg.f32IDcLd = g_sEstimRLInitFMSTR.f32IDcLd;
          g_sEstimRLInitCfg.f32IDcLq = g_sEstimRLInitFMSTR.f32IDcLq;
          g_sEstimRLInitCfg.f32IDcNegMax = g_sEstimRLInitFMSTR.f32IDcNegMax;
          g_sEstimRLInitCfg.u16LdqNumMeas = NUM_MEAS;
          
          g_sEstimRLCtrlRun.pLdqTable = f32LdqTable;
          g_sEstimRLCtrlRun.f32IDcDReq = 0;
          g_sEstimRLCtrlRun.f32IDcQReq = 0;
          g_sEstimRLCtrlRun.f32IAcReq = 0;
          g_sEstimRLCtrlRun.u16FAc = 0U;               
          g_sEstimRLCtrlRun.u8LdqSwitch = 0U;
          break;
      case 3:
          /* Mode 3 */
          
          g_sEstimRLInitCfg.f32IDcMax = 0;
          g_sEstimRLInitCfg.f32IDcLd = 0;
          g_sEstimRLInitCfg.f32IDcLq = 0;
          g_sEstimRLInitCfg.f32IDcNegMax = 0;
          g_sEstimRLInitCfg.u16LdqNumMeas = 1U;
          
          g_sEstimRLCtrlRun.pLdqTable = 0;
          g_sEstimRLCtrlRun.f32IDcDReq = 0;
          g_sEstimRLCtrlRun.f32IDcQReq = 0;
          g_sEstimRLCtrlRun.f32IAcReq = 0;
          g_sEstimRLCtrlRun.u16FAc = 1U;                /* Set frequency greater than zero to avoid returning error from EstimRL. */
          g_sEstimRLCtrlRun.u8LdqSwitch = 0U;
          break;
      default:
          /* Mode 0 */
          g_sEstimRLInitCfg.f32IDcMax = g_sEstimRLInitFMSTR.f32IDcNom;
          g_sEstimRLInitCfg.f32IDcLd = 0;
          g_sEstimRLInitCfg.f32IDcLq = 0;
          g_sEstimRLInitCfg.f32IDcNegMax = 0;
          g_sEstimRLInitCfg.u16LdqNumMeas = 0U;
          
          g_sEstimRLCtrlRun.pLdqTable = 0;
          g_sEstimRLCtrlRun.f32IDcDReq = 0;
          g_sEstimRLCtrlRun.f32IDcQReq = 0;
          g_sEstimRLCtrlRun.f32IAcReq = 0;
          g_sEstimRLCtrlRun.u16FAc = 0U;               
          g_sEstimRLCtrlRun.u8LdqSwitch = 0U;                
          break;
    }

    eEstimRetValInit = MCAA_EstimRLInit_F16(F_SAMPLING, &g_sEstimRLStruct, g_sEstimRLInitCfg);

    /* acknowledge that the state machine can proceed into RL state */ 
    switch(eEstimRetValInit)
    {
      case RET_INIT_OK:      
          /* Initialization of the EstimRL was successful */
          g_sMID.eMIDState = kMID_RL;
          break;
      default:
          g_sMID.ui16FaultMID |= MID_ESTIMRL_INIT_ERROR;
          MID_TransAll2Fault();            
          break;
    }
}

/*!
 * @brief MID START to MECHPARAM transition
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_TransStart2MechParam(void)
{
    /* Type the code to do when going from the Start to the MechParam state */

    /* acknowledge that the state machine can proceed into MechParam state */
    g_sMID.eMIDState = kMID_MechParam;
}

/*!
 * @brief MID START to PP transition
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_TransStart2Pp(void)
{
    /* Type the code to do when going from the START to the PP state */
    g_sMID.sMIDPp.bActive       = FALSE;
    g_sMID.sMIDPp.ui16PpDetermined = 0U;

    /* Acknowledge that the system can proceed into the PP state */
    g_sMID.eMIDState = kMID_Pp;
}

/*!
 * @brief MID ALL to STOP transition
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_TransAll2Stop(void)
{
    /* Type the code to do when going from the All states to the Stop state */
      
    /* Acknowledge that the system can proceed into the STOP state */
    
    /* Type the code to do when going from the RUN CALIB to the RUN READY sub-state */
    /* Set off measurement */
    g_sMID.bMIDOnOff = 0;
    g_sMID.bMIDRun = FALSE;

    /* set 50% PWM duty cycle */
    g_sMidDrive.sFocPMSM.sDutyABC.f16A = FRAC16(0.5);
    g_sMidDrive.sFocPMSM.sDutyABC.f16B = FRAC16(0.5);
    g_sMidDrive.sFocPMSM.sDutyABC.f16C = FRAC16(0.5);     
    
    M1_MCDRV_PWM3PH_DIS(&g_sM1Pwm3ph);
    
    g_sMID.eMIDState = kMID_Stop;
}

/*!
 * @brief MID ALL to FAULT transition
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_TransAll2Fault(void)
{
    /* Type the code to do when going from the All states to the FAULT state */
    
    /* Acknowledge that the system can proceed into the FAULT state */
    g_sMID.bMIDOnOff = 0;
    g_sMID.bMIDRun = FALSE;

    /* set 50% PWM duty cycle */
    g_sMidDrive.sFocPMSM.sDutyABC.f16A = FRAC16(0.5);
    g_sMidDrive.sFocPMSM.sDutyABC.f16B = FRAC16(0.5);
    g_sMidDrive.sFocPMSM.sDutyABC.f16C = FRAC16(0.5);     
    
    M1_MCDRV_PWM3PH_DIS(&g_sM1Pwm3ph);
    
    g_sMID.eMIDState = kMID_Fault;
}

/*!
 * @brief MID STOP to START transition
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_TransStop2Start(void)
{
  /* Type the code to do when going from the STOP to the START state */

  /* 50% duty cycle */
  g_sMidDrive.sFocPMSM.sDutyABC.f16A = FRAC16(0.5);
  g_sMidDrive.sFocPMSM.sDutyABC.f16B = FRAC16(0.5);
  g_sMidDrive.sFocPMSM.sDutyABC.f16C = FRAC16(0.5);

  /* PWM duty cycles calculation and update*/
  M1_MCDRV_PWM3PH_SET(&g_sM1Pwm3ph);

  /* Clear offset filters */
  M1_MCDRV_CURR_3PH_CALIB_INIT(&g_sM1AdcSensor);

  /* Enable PWM output */
  M1_MCDRV_PWM3PH_EN(&g_sM1Pwm3ph);

  /* pass calibration routine duration to state counter*/
  g_sMidDrive.ui16CounterState = M1_CALIB_DURATION * 10;
  
  g_sMID.bMIDRun = TRUE;
  
  g_sMID.eMIDState = kMID_Start;
}

/*!
 * @brief Fault detention routine - check various faults
 *
 * @param void  No input parameter
 *
 * @return None
 */
static void MID_FaultDetection(void)
{
    /* Clearing actual faults before detecting them again  */
    /* Clear all pending faults */
    MID_FAULT_CLEAR_ALL(g_sMidDrive.sFaultIdPending);

    /* Clear fault captured manually if required. */
    if (g_sMidDrive.bFaultClearMan)
    {
        /* Clear fault captured */
        MID_FAULT_CLEAR_ALL(g_sMidDrive.sFaultIdCaptured);
        g_sMidDrive.bFaultClearMan = FALSE;
    }

    /* Fault:   DC-bus over-current */
    if (M1_MCDRV_PWM3PH_FLT_GET(&g_sM1Pwm3ph))
    {
        MID_FAULT_SET(g_sMidDrive.sFaultIdPending, MID_FAULT_I_DCBUS_OVER);
    }

    /* Fault:   DC-bus over-voltage */
    if (g_sMidDrive.sFocPMSM.f16UDcBusFilt > g_sMidDrive.sFaultThresholds.f16UDcBusOver)
    {
        MID_FAULT_SET(g_sMidDrive.sFaultIdPending, MID_FAULT_U_DCBUS_OVER);
    }

    /* Fault:   DC-bus under-voltage */
    if (g_sMidDrive.sFocPMSM.f16UDcBusFilt < g_sMidDrive.sFaultThresholds.f16UDcBusUnder)
    {
        MID_FAULT_SET(g_sMidDrive.sFaultIdPending, MID_FAULT_U_DCBUS_UNDER);
    }

    /* pass fault to Fault ID Captured */
    g_sMidDrive.sFaultIdCaptured |= g_sMidDrive.sFaultIdPending;
}

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * MID fast-loop process function.
 */
void MID_ProcessFast_FL(void)
{
    /* get all adc samples - DC-bus voltage, current, bemf and aux sample */
    M1_MCDRV_ADC_GET(&g_sM1AdcSensor);

    /* Sampled DC-Bus voltage filter */
    g_sMidDrive.sFocPMSM.f16UDcBusFilt =
        GDFLIB_FilterIIR1_F16(g_sMidDrive.sFocPMSM.f16UDcBus, &g_sMidDrive.sFocPMSM.sUDcBusFilter);
    
    /* detect fault */
    MID_FaultDetection();
    
    /* If a fault occurred go to Fault. Higher priority then stopping by the user. */
    if (g_sMidDrive.sFaultIdPending != 0U)
    {
      MID_TransAll2Fault();
    }    
    else if((g_sMID.bMIDRun == TRUE) && (g_sMID.bMIDOnOff == 0))
    {
      /* MID is stopped by user */
      MID_TransAll2Stop();
    }
    
    /* Execute the MID state machine. */
    g_MID_SM_STATE_TABLE[g_sMID.eMIDState]();
    
    /* PWM peripheral update */
    M1_MCDRV_PWM3PH_SET(&g_sM1Pwm3ph);

    /* set current sensor for  sampling */
    M1_MCDRV_CURR_3PH_CHAN_ASSIGN(&g_sM1AdcSensor);
}

/*!
 * MID initialization function.
 */
void MID_Init(void)
{
    /* Type the code to do when in the INIT routine */
  
    /* Current controllers */
    g_sMidDrive.sFocPMSM.sIdPiParams.a32PGain    = MID_KP_GAIN;
    g_sMidDrive.sFocPMSM.sIdPiParams.a32IGain    = MID_KI_GAIN;
    g_sMidDrive.sFocPMSM.sIdPiParams.f16InErrK_1 = 0;
    g_sMidDrive.sFocPMSM.sIdPiParams.f16UpperLim = FRAC16(1.0);
    g_sMidDrive.sFocPMSM.sIdPiParams.f16LowerLim = FRAC16(-1.0);
    g_sMidDrive.sFocPMSM.sIdPiParams.bLimFlag    = FALSE;

    g_sMidDrive.sFocPMSM.sIqPiParams.a32PGain    = MID_KP_GAIN;
    g_sMidDrive.sFocPMSM.sIqPiParams.a32IGain    = MID_KI_GAIN;
    g_sMidDrive.sFocPMSM.sIqPiParams.f16InErrK_1 = 0;
    g_sMidDrive.sFocPMSM.sIqPiParams.f16UpperLim = FRAC16(1.0);
    g_sMidDrive.sFocPMSM.sIqPiParams.f16LowerLim = FRAC16(-1.0);
    g_sMidDrive.sFocPMSM.sIqPiParams.bLimFlag    = FALSE;

    g_sMidDrive.sFocPMSM.ui16SectorSVM     = 4;
    g_sMidDrive.sFocPMSM.f16DutyCycleLimit = M1_CLOOP_LIMIT;
    
    g_sMidDrive.sFocPMSM.f16UDcBus                     = 0;
    g_sMidDrive.sFocPMSM.f16UDcBusFilt                 = 0;
    g_sMidDrive.sFocPMSM.sUDcBusFilter.sFltCoeff.f32B0 = M1_UDCB_IIR_B0;
    g_sMidDrive.sFocPMSM.sUDcBusFilter.sFltCoeff.f32B1 = M1_UDCB_IIR_B1;
    g_sMidDrive.sFocPMSM.sUDcBusFilter.sFltCoeff.f32A1 = M1_UDCB_IIR_A1;
    /* Filter init not to enter to fault */
    g_sMidDrive.sFocPMSM.sUDcBusFilter.f16FltBfrX[0] =
        (frac16_t)((M1_U_DCB_UNDERVOLTAGE / 2) + (M1_U_DCB_OVERVOLTAGE / 2));
    g_sMidDrive.sFocPMSM.sUDcBusFilter.f32FltBfrY[0] =
        (frac32_t)((uint32_t)((M1_U_DCB_UNDERVOLTAGE / 2) + (M1_U_DCB_OVERVOLTAGE / 2)) << 16);
    
    g_fltMIDcurrentScale    = M1_I_MAX;
    g_fltMIDDCBvoltageScale = M1_U_DCB_MAX;
    g_fltMIDparamScale	    = (float)(M1_U_MAX / M1_I_MAX);
    g_fltMIDspeedScale      = M1_N_MAX;
    
    /* fault set to init states */
    MID_FAULT_CLEAR_ALL(g_sMidDrive.sFaultIdCaptured);
    MID_FAULT_CLEAR_ALL(g_sMidDrive.sFaultIdPending);

    /* fault thresholds */
    g_sMidDrive.sFaultThresholds.f16UDcBusOver     = M1_U_DCB_OVERVOLTAGE;
    g_sMidDrive.sFaultThresholds.f16UDcBusUnder    = M1_U_DCB_UNDERVOLTAGE;
    
    /* Init sensors/actuators pointers */
    /* For PWM driver */
    g_sM1Pwm3ph.psUABC = &(g_sMidDrive.sFocPMSM.sDutyABC);
    
    /* For ADC driver */
    g_sM1AdcSensor.pf16UDcBus     = &(g_sMidDrive.sFocPMSM.f16UDcBus);
    g_sM1AdcSensor.psIABC         = &(g_sMidDrive.sFocPMSM.sIABC);
    g_sM1AdcSensor.pui16SVMSector = &(g_sMidDrive.sFocPMSM.ui16SectorSVM);
    g_sM1AdcSensor.pui16AuxChan   = &(g_sMidDrive.f16AdcAuxSample);
    
    /* Set the initial MID SM variables. */
    g_sMID.eMIDState = kMID_Stop;
    g_sMID.bMIDRun = FALSE; 
    
    /* Pp measurement */
    g_sMID.sMIDPp.pf16PosEl = &(g_sMidDrive.sFocPMSM.f16PosElExt);
    g_sMID.sMIDPp.pf16IdReq = &(g_sMidDrive.sFocPMSM.sIDQReq.f16D);
    g_sMID.sMIDPp.f16IdReqOpenLoop = MID_PP_ID_REQ;
    g_sMID.sMIDPp.f16SpeedElReq = MID_PP_SPEED_EL;
    
    /* enable passing external electrical position to FOC */
    g_sMidDrive.sFocPMSM.bPosExtOn = TRUE;

    /* During the measurement motor is driven open-loop */
    g_sMidDrive.sFocPMSM.bOpenLoop = TRUE;
}

/*!
 * Function returns actual state of MID.
 */
uint16_t MID_GetActualState(void)
{
  return ((uint16_t)g_sMID.eMIDState);
}

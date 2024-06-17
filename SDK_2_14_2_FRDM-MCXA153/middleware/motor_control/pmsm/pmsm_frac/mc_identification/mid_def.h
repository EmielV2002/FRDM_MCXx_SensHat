/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MID_DEF_H_
#define _MID_DEF_H_

#include "m1_pmsm_appconfig.h"
#include "mc_periph_init.h"
#include "pmsm_control.h"

/* RTCESL fix libraries. */
#include "mlib.h"
#include "gflib.h"
#include "amclib.h"

//Motor Identification Module           
//----------------------------------------------------------------------
#define M1_CHAR_NUMBER_OF_POINTS_BASE      (6)
#define M1_CHAR_CURRENT_POINT_NUMBERS      (65)
#define M1_CHAR_NUMBER_OF_POINTS_HALF      (32)
#define M1_TIME_50MS                       (500)
#define M1_TIME_100MS                      (1000)
#define M1_TIME_300MS                      (3000)
#define M1_TIME_600MS                      (6000)
#define M1_TIME_1200MS                     (12000)
#define M1_TIME_2400MS                     (24000)
#define M1_K_RESCALE_DCB_TO_PHASE_HALF     FRAC16(0.866096866096866)
#define M1_K_ANGLE_INCREMENT               FRAC16(0.2)
#define M1_INV_MOD_INDEX                   FRAC16(0.4999587446189769)
#define M1_K_I_50MA                        FRAC16(0.006060606060606061)

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MID Fault numbers */
#define MID_ESTIMRL_INIT_ERROR (1U)
#define MID_ESTIMRL_RUN_ERROR  (2U)

/* Current controllers' coefficients ensuring slow response for variable parameters */
#define MID_KP_GAIN ACC32(0.01935102757040625)
#define MID_KI_GAIN ACC32(0.0036080115416992)

/* Speed minimal ramp */
#define MID_SPEED_RAMP_UP FRAC16(0.000060606061)
#define MID_SPEED_RAMP_DOWN FRAC16(0.000060606061)

/* Pp assistant variables */
#define MID_PP_ID_REQ FRAC16(0.1)
#define MID_PP_SPEED_EL FRAC16(0.1)

/* Time quantities in units of fast loop ticks. */
#define MID_TIME_2400MS (uint32_t)(2.4F * (float)(M1_PWM_FREQ / M1_FOC_FREQ_VS_PWM_FREQ))

/* Maximal measuring signal levels. */
#define MID_I_MEAS_MAX  M1_I_MAX                                /* Current sensing HW scale [A]. */
#define SCALE(x)        (FRAC32(x / MID_I_MEAS_MAX))

/* MID parameters measurement options. */
#define MID_EL_PP (1UL <<  0U)
#define MID_EL_RL (1UL <<  1U)
#define MID_EL_ME (1UL <<  2U)

/* EstimRL measurement parameters. */ 
#define F_SAMPLING 		M1_PWM_FREQ             /* EstimRL - Sampling frequency [Hz]. Maximum is 10 kHz. */
#define NUM_MEAS 		20U                     /* EstimRL - Number of measurement. */
#define I_NOMINAL 		5.0F                    /* EstimRL - Nominal current [A]. */
#define I_POSMAX 		6.0F                    /* EstimRL - Maximum positive current [A]. */
#define I_NEGMAX 		-6.0F                   /* EstimRL - Minimum positive current [A]. */
#define I_LD			0.0F                    /* EstimRL - Current to determine inductance in d-axis [A]. */
#define I_LQ			I_NOMINAL               /* EstimRL - Current to determine inductance in q-axis [A]. */

/* Sets the fault bit defined by faultid in the faults variable */
#define MID_FAULT_SET(faults, faultid) ((faults) |= (((middef_fault_t)1U) << (faultid)))

/* Clears all fault bits in the faults variable */
#define MID_FAULT_CLEAR_ALL(faults) ((faults) = 0U)

/* Check if a fault bit is set in the faults variable, 0 = no fault */
#define MID_FAULT_ANY(faults) ((faults) > 0U)

#define MID_FAULT_I_DCBUS_OVER (0U)  /* OverCurrent fault flag */
#define MID_FAULT_U_DCBUS_UNDER (1U) /* Undervoltage fault flag */
#define MID_FAULT_U_DCBUS_OVER (2U)  /* Overvoltage fault flag */

/* Device fault typedef */
typedef uint16_t middef_fault_t;

/* Device fault thresholds */
typedef struct _middef_fault_thresholds_t
{
    frac16_t f16UDcBusOver;     /* DC bus over voltage level */
    frac16_t f16UDcBusUnder;    /* DC bus under voltage level */
} middef_fault_thresholds_t;

/* Pointer to function with a pointer to state machine control structure. */
typedef void (*mid_pfcn_void_pms)(void);

typedef struct
{
    uint16_t bActive;           /* Indicates whether Ke is being measured (true) or not (false) */
    uint16_t ui16PpDetermined;  /* Indicates whether the user already set pp in MCAT (true) or not yet (false) */
    uint16_t ui16WaitingSteady; /* Indicates that motor is waiting in steady state (when electrical position is zero) */
    uint16_t ui16LoopCounter;   /* Serves for timing to determine e.g. 300ms */
    frac16_t *pf16PosEl;        /* Pointer to electrical position for Park transformations */
    frac16_t *pf16IdReq;        /* Pointer to required current Id (PI current controller's input) */
    frac16_t f16PosElCurrent;   /* Current value of electrical position */
    frac16_t f16PosElLast;      /* Last value of electrical position */
    frac16_t f16IdReqOpenLoop;  /* Openloop current */
    frac16_t f16SpeedElReq;     /* Required Electrical Speed */
    frac16_t f16SpeedElRamp;    /* Ramped f16SpeedElReq, this speed is integrated to get position */
    GFLIB_RAMP_T_F16 sSpeedElRampParam;      /* Ramp Up + Down coefficients for f16Speed */
    GFLIB_INTEGRATOR_T_A32 sSpeedIntegrator; /* Speed integrator coefficients */
} mid_get_pp_a1_t;

/* Measurement type enumeration. */
typedef enum _mid_meas_type_t
{
    kMID_PolePairs        = 0U,
    kMID_ElectricalParams = 1U,
    kMID_MechanicalParams = 2U,
} mid_meas_type_t;

/* States of machine enumeration. */
typedef enum _mid_sm_app_state_t{
    kMID_Pp            = 0U,
    kMID_RL            = 1U,
    kMID_MechParam     = 2U,
    kMID_Start         = 3U,
    kMID_Stop          = 4U,
    kMID_Fault         = 5U,
} mid_sm_app_state_t;

/* Measurement global structure. */
typedef struct _mid_struct_t
{
    mid_get_pp_a1_t     sMIDPp;             /* Input structure for MID_getPp() */
    mid_meas_type_t     eMeasurementType;   /* Measurement type. */
    mid_sm_app_state_t  eMIDState;          /* MID state machine actual state. */
    frac32_t            f32Rs;              /* Estimated resistance. */
    frac32_t            f32Ld;              /* Estimated d-axis inductance. */
    frac32_t            f32Lq;              /* Estimated q-axis inductance. */
    uint32_t            ui32FinishedMeas;   /* Array for finished eletrical measurements. */
    uint16_t            ui16FaultMID;       /* MID fault code. */
    uint16_t            ui16WarnMID;        /* MID warning code. */
    bool_t              bMIDRun;            /* 1 - MID is running, 0 - MID is in STOP/FAULT state */
    bool_t              bMIDOnOff;          /* Measurement On/Off */
} mid_struct_t;

/* EstimRL config params structure. */
typedef struct _rl_estim_cfg_params_t
{
  frac32_t f32IDcNom;            /* Scaled nominal DC current [A]. */
  frac32_t f32IDcPosMax;         /* Scaled maximum DC current [A]. */
  frac32_t f32IDcNegMax;         /* Scaled maximum allowed negative d-axis DC current [A]. The value of f32IDcNegMax must be negative or zero. */
  frac32_t f32IDcLd;             /* Scaled DC current used for Ld measurement [A]. */
  frac32_t f32IDcLq;             /* Scaled DC current used for Lq measurement [A]. */
  uint8_t  u8ModeEstimRL;        /* Selected EstimRL mode: 0, 1, 2, 3. */
}rl_estim_cfg_params_t;

/* FOC global structure */
typedef struct _middef_pmsm_t
{
    mcs_pmsm_foc_t sFocPMSM;                   /* Field Oriented Control structure */
    middef_fault_t sFaultIdCaptured;            /* Captured faults (must be cleared manually) */
    middef_fault_t sFaultIdPending;             /* Fault pending structure */
    middef_fault_thresholds_t sFaultThresholds; /* Fault thresholds */
    frac16_t f16AdcAuxSample;                      /* Auxiliary ADC sample  */
    uint16_t ui16CounterState;                     /* Main state counter */
    bool_t bFaultClearMan;                         /* Manual fault clear detection */
} middef_pmsm_t;

#endif /* _MID_DEF_H_ */

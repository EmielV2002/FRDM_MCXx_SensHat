/******************************************************************************
*
 * Copyright 2022 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/
/**
*
* @file       MCAA_EstimRL.h
*
* @version    1.0.0.0
*
* @date       16-November-2022
*
* @brief      Header file for mcaa_estimrl function
*
******************************************************************************/
#ifndef MCAA_ESTIMRL_H_
#define MCAA_ESTIMRL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Includes
*******************************************************************************/
#include "gflib.h"
#include "gmclib.h"
#include "gdflib.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define MCAA_EstimRLInit_F16_C(u16SamplingFreq, psParam, psCtrl)               \
        MCAA_EstimRLInit_F16_FC(u16SamplingFreq, psParam, psCtrl)
#define MCAA_EstimRL_F16_C(f16UDcBus, psIAlBeFbck, psCtrl, sParam, psUAlBeReq) \
        MCAA_EstimRL_F16_FC(f16UDcBus, psIAlBeFbck, psCtrl, sParam, psUAlBeReq)

/*******************************************************************************
* Types
*******************************************************************************/
/* MCAA_EstimRLInit return value enum. */
typedef enum
{
  RET_INIT_OK = 0,              /* Initialization successful. */
  RET_INIT_ERROR = 1,           /* Invalid inputs. */
} MCAA_ESTIMRLINIT_RET_T;

/* MCAA_EstimRL return value enum. */
typedef enum
{
  RET_IN_PROGRESS = 0,          /* Parameter estimation is in progress. The MCAA_EstimRL must be called again in the next sampling period. */
  RET_DONE = 1,                 /* Parameter estimation has finished. */
  RET_ERROR = 2,                /* Parameter estimation has failed. */
} MCAA_ESTIMRL_RET_T;

/* MCAA_EstimRL internal state enum. */
typedef enum
{
  STATE_UNINITIALIZED = 0,      /* RL estimator is not initialized. */
  STATE_MEAS_LD,                /* Measurement of d-axis inductance. */
  STATE_MEAS_LQ,                /* Measurement of q-axis inductance. */
  STATE_LD_TO_LQ,               /* Transition between axes. */
  STATE_POSTPROCESS_LD,         /* Postprocessing of accumulated values for d-axis measurement. */
  STATE_POSTPROCESS_LQ,         /* Postprocessing of accumulated values for q-axis measurement. */
  STATE_UNWIND,                 /* Gradual turn-off of the DC current at the end of measurement. */
  STATE_DONE,                   /* Measurement finished. */
  STATE_ERROR,                  /* Failure. */
} MCAA_ESTIMRL_STATE_T;

/* Internal state of the RL estimator structure. */
typedef struct
{
  MCAA_ESTIMRL_STATE_T pState;  /* RL estimator state */
  uint16_t u16SamplingFreq;     /* Sampling frequency [Hz] */
  frac32_t f32FAc;              /* Normalized fraction of the AC current frequency to be integrated in each iteration */
  frac32_t f32IDcMax;           /* Maximum DC of current */
  frac32_t f32IDcNegMax;        /* Maximum allowed negative DC current */
  frac32_t f32IAcReq;           /* Amplitude of the required AC current */
  int32_t u32IAcReqInv;         /* Inverse of the amplitude of the required AC current */
  frac32_t f32IDcDReq;          /* Required DC current in D axis */
  frac32_t f32IDcDReqk_1;       /* Required DC current in previous step in D axis */
  frac32_t f32IDcQReq;          /* Required DC current in Q axis */
  frac32_t f32IDcQReqk_1;       /* Required DC current in previous step in Q axis */
  frac32_t f32IDcReqStep;       /* Measurement step of the required DC current */
  frac32_t f32IAlBeEstErr;              /* Current estimation error */
  GMCLIB_2COOR_ALBE_T_F32 pIAlBeAc;     /* Alpha/beta components of the estimated AC current */
  GMCLIB_2COOR_SINCOS_T_F16 pSinCosEst; /* Sine/Cosine of the estimated current phase */
  GMCLIB_2COOR_DQ_T_F32 pIDQAcRaw;      /* d/q components of the estimated AC current */
  GMCLIB_2COOR_DQ_T_F32 pIDQAcFilt;     /* Filtered d/q components of the estimated AC current */
  GDFLIB_FILTER_MA_T_A32 pDAxisFilter;  /* Parameters structure of the MA filter in the d axis in the PLL */
  GDFLIB_FILTER_MA_T_A32 pQAxisFilter;  /* Parameters structure of pDcFilter in the PLL */
  GDFLIB_FILTER_MA_T_A32 pDcFilter;     /* Parameters structure of the MA filter filtering the DC current in PLL */
  GDFLIB_FILTER_MA_T_A32 pStdFilter;    /* Parameters structure of the MA filter filtering the standard deviation of current noise */
  frac32_t f32PLLPropGain;              /* Gain of the P controller in the PLL */
  frac32_t f32ThAc;                     /* High-frequency AC voltage phase; <-PI, PI) range normalized into <-1, 1) */
  frac32_t f32ThEst;                    /* Estimated AC current phase; <-PI, PI) range normalized into <-1, 1) */
  frac32_t f32IDcD;                     /* Estimated DC current in D axis */
  frac32_t f32IDcQ;                     /* Estimated DC current in Q axis */
  int32_t i32IDcDAvg;                   /* Averaging accumulator for the estimated DC current in D axis */
  frac32_t f32IDcDAvgk_1;               /* Averaged estimated DC current in D axis in the last step */
  frac32_t f32PhComp;                   /* Compensation of phase error due to sampling lag, scaled by phi */
  int32_t i32ConvDetState;              /* Convergence detector state */
  int32_t i32ConvDetStateLimL;          /* Convergence detector lower limit */
  int32_t i32ConvDetStateLimH;          /* Convergence detector coarse/fine tracking threshold */
  uint32_t u32ConvDetToutCnt;           /* Convergence detector timeout counter */
  uint32_t u32ConvDetToutReload;        /* Convergence detector counter reload value */
  uint32_t u32AvgCnt;                   /* Noise averaging counter */
  uint32_t u32AvgN;                     /* Noise averaging window length */
  uint32_t u32AvgN1;                    /* Noise averaging window length in the first iteration */
  uint32_t u32AvgCntMax;                /* Maximum length of the noise averaging window */
  frac32_t f32PhAvg;                    /* Averaging accumulator for phase shift, scaled by phi */
  frac32_t f32IAcAvg;                   /* Averaging accumulator for AC current */
  frac32_t f32UAcAvg;                   /* Averaging accumulator for AC voltage */
  int32_t i32AvgMul;                    /* Averaging window length multiplier */
  int32_t i32AvgMin;                    /* Averaging window minimum length */
  uint16_t u16LdqNumMeas;               /* Number of Ld/Lq table measurements */
  uint32_t u32TblCounter;               /* Ld/Lq table counter */
  uint8_t u8RotorFixed;                 /* 0 == rotor is not mechanically fixed, 1 == rotor is mechanically fixed */
  frac32_t f32IDcRampCoef;              /* DC current ramp coefficient */
  frac32_t f32IDcRampCoefMax;           /* Maximum DC current ramp coefficient */
  frac32_t f32IDcDRampAcc;              /* DC current ramp accumulator in D axis */
  frac32_t f32IDcQRampAcc;              /* DC current ramp accumulator in Q axis */
  frac32_t f32UDcDAcc;                  /* DC voltage integrator in D axis */
  int32_t i32UDcDAccAvg;                /* Averaging accumulator for DC voltage in D axis */
  frac32_t f32UDcDAccAvgk_1;            /* Averaged DC voltage in D axis in previous step */
  frac32_t f32UDcQAcc;                  /* DC voltage integrator in Q axis */
  frac32_t f32UAcAcc;                   /* AC voltage integrator  */
  frac32_t f32UDcKi;                    /* DC voltage integrator gain */
  frac32_t f32UAcKi;                    /* AC voltage integrator gain */
  frac32_t f32UDcKiNominal;             /* Nominal DC voltage integrator gain */
  frac32_t f32UAcKiNominal;             /* Nominal AC voltage integrator gain */
  uint16_t u16RampSteady;               /* Indicator of the steady state of the voltage controller ramp */
  uint16_t u16DcbLimitFlag;             /* DC bus voltage limitation indicator */
  uint16_t u16DcbLimitFlagk_1;          /* DC bus voltage limitation indicator in previous step */
  uint16_t u16DcbLimitCnt;              /* DC bus voltage limitation counter */
  frac32_t f32PhaseShift;               /* Voltage/current phase shift, scaled by phi */
  frac32_t f32UdcHalf;                  /* Minimum DC bus voltage, halved and divided by modulation index */
  frac32_t f32FsInv;                    /* Inverse of the sampling frequency */
  frac32_t f32ImpConst;                 /* Constant used for calculation of impedance */
  frac32_t f32Ldk_1;                    /* Estimated d-axis inductance in previous step */
  frac32_t f32Lqk_1;                    /* Estimated q-axis inductance in previous step */
  frac32_t f32IDcLd;                    /* DC current level to use for measurement of scalar Ld */
  frac32_t f32IDcLq;                    /* DC current level to use for measurement of scalar Lq */
  frac32_t f32UdtAcc;                   /* Dead time voltage drop averaging accumulator */
  frac32_t f32RAcc;                     /* Resistance averaging accumulator, format Q33.31 */
  uint16_t u16LdqNumMeasR;              /* Number of measurements of R */
  frac32_t f32UDcBusMax;                /* Maximum observed DC bus voltage */
  frac32_t f32CDFineThr;                /* Convergence detector fine threshold */
  frac32_t f32CDCoarseThr;              /* Convergence detector coarse threshold */
  uint16_t u16ScComp;                   /* Scaling compensation flag */
} MCAA_ESTIMRL_INNERST_T_F32;

/* Estimator configuration structure. */
typedef struct
{
  frac32_t f32Ld;       /* Estimated d-axis inductance at zero DC current scaled in the [-1, 1) normalized range. Provided result must be multiplied by \f$U_{MAX}/I_{MAX}\f$ to get the inductance in [H]. */
  frac32_t f32Lq;       /* Estimated q-axis inductance at maximal DC current scaled in the [-1, 1) normalized range. Provided result must be multiplied by \f$U_{MAX}/I_{MAX}\f$ to get the inductance in [H]. */
  frac32_t f32R;        /* Estimated resistance scaled in the [-1, 1) normalized range. Provided result must be multiplied by \f$U_{MAX}/I_{MAX}\f$ to get the resistance in [Ohm]. */
  frac32_t f32Udt;      /* Estimated dead time voltage drop of the power stage scaled in the [-1, 1) normalized range. Provided result must be multiplied by \f$U_{MAX}\f$ to get the voltage in [V]. */
  MCAA_ESTIMRL_INNERST_T_F32 pInnerState;  /* Internal state of the RL estimator. No user-editable values. */
} MCAA_ESTIMRL_T_F32;

/* Initialization parameters of the RL estimator structure. */
typedef struct
{
  frac32_t f32IDcMax;           /* Maximum DC current. The value is scaled by \f$I_{MAX}.\f$ */
  uint16_t u16LdqNumMeas;       /* Number of measured d-axis DC current levels. If this parameter is not 1, the function will perform automatic measurement for a range of DC currents. Otherwise, only one measurement will be performed. */
  frac32_t f32IDcLd;            /* DC current used for Ld measurement. The value is scaled by \f$I_{MAX}.\f$ */
  frac32_t f32IDcLq;            /* DC current used for Lq measurement. The value is scaled by \f$I_{MAX}.\f$ */
  frac32_t f32IDcNegMax;        /* Maximum allowed negative d-axis DC current. The value is scaled by \f$I_{MAX}.\f$ Note that too large negative DC current can cause permanent damage to the motor. Setting f32IDcNegMax to a safe value prevents irreversible demagnetization of the PMSM's magnets. The value of f32IDcNegMax must be negative or zero. */
} MCAA_ESTIMRL_INIT_T_F32;

/* Run-time parameters of the RL estimator structure. */
typedef struct
{
  frac32_t *pLdqTable;          /* Pointer to a table with measured values. The table has 3 rows and u16LdqNumMeas columns stored in column-major order. Parameter u16LdqNumMeas of the init function MCAA_EstimRLInit_F16 determines the number of used DC current levels. The array pointed to by pLdqTable must provide room for 3*u16LdqNumMeas 32-bit values. The first row of the table contains the DC current levels, the second row contains the estimated d-axis inductances, and the third row contains the estimated q-axis inductances. */
  frac32_t f32IDcDReq;          /* Required DC current in the d-axis. The value is scaled by \f$I_{MAX}.\f$ */
  frac32_t f32IDcQReq;          /* Required DC current in the q-axis. The value is scaled by \f$I_{MAX}.\f$ */
  frac32_t f32IAcReq;           /* Amplitude of the required AC current. The value is scaled by \f$I_{MAX}.\f$ */
  uint16_t u16FAc;              /* AC current frequency [Hz] */
  uint8_t u8LdqSwitch;          /* Switches between Ld (u8LdqSwitch = 0) and Lq (u8LdqSwitch = 1) measurement */
} MCAA_ESTIMRL_RUN_T_F32;

/****************************************************************************
* Exported function prototypes
****************************************************************************/
extern MCAA_ESTIMRLINIT_RET_T MCAA_EstimRLInit_F16_FC(uint16_t u16SamplingFreq,
                                                   MCAA_ESTIMRL_T_F32 *const pCtrl,
                                                   MCAA_ESTIMRL_INIT_T_F32 pParam);
extern MCAA_ESTIMRL_RET_T MCAA_EstimRL_F16_FC(frac16_t f16UDcBusMeas,
                                           const GMCLIB_2COOR_ALBE_T_F16 *const pIAlBeFbck,
                                           MCAA_ESTIMRL_T_F32 *const pCtrl,
                                           MCAA_ESTIMRL_RUN_T_F32 pParam,
                                           GMCLIB_2COOR_ALBE_T_F16 *const pUAlBeReq);

/****************************************************************************
* Inline functions
****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif/* MCAA_ESTIMRL_H */

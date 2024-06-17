/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MC_PERIPH_INIT_H_
#define _MC_PERIPH_INIT_H_

#include "mlib.h"
#include "gflib.h"
#include "gmclib.h"

/******************************************************************************
 * Timing
 ******************************************************************************/
/* PWM frequency in Hz*/
#define M1_PWM_FREQ             (10000U)
/* PWM vs. Fast control loop ratio */
#define M1_FOC_FREQ_VS_PWM_FREQ (1U)
/* Fast loop frequency in Hz */
#define M1_FAST_LOOP_FREQ       (M1_PWM_FREQ / M1_FOC_FREQ_VS_PWM_FREQ)
/* Slow control loop frequency */
#define M1_SLOW_LOOP_FREQ       (1000U)
/* Slow Loop modulo = FTM_input_clock / M1_SLOW_LOOP_FREQ */
#define M1_SLOW_LOOP_MODULO     (75000000UL / M1_SLOW_LOOP_FREQ)
/* Fast loop period */
#define M1_FAST_LOOP_TS         ((float_t)1.0 / (float_t)(M1_FAST_LOOP_FREQ))
/* Slow loop period */
#define M1_SLOW_LOOP_TS         ((float_t)1.0 / (float_t)(M1_SLOW_LOOP_FREQ))

/******************************************************************************
* Output control
******************************************************************************/
/* DC bus braking resistor control */
#define M1_BRAKE_SET()          
#define M1_BRAKE_CLEAR()        

/* DC bus braking threshold hysteresis */
#define U_DCB_TRIP                  FRAC16(0.5)         /* Half of the maximum DC bus voltage. */
#define M1_U_DCB_HYSTERESIS         (0.05F)
#define M1_U_DCB_UPPER_THRESHOLD    (frac16_t)((U_DCB_TRIP) * (1.0F + (M1_U_DCB_HYSTERESIS)))
#define M1_U_DCB_LOWER_THRESHOLD    (frac16_t)((U_DCB_TRIP) * (1.0F - (M1_U_DCB_HYSTERESIS)))

/******************************************************************************
 * MC driver macro definition - only template
 ******************************************************************************/
/******************************************************************************
 * Define motor ADC control functions
 ******************************************************************************/
#define M1_MCDRV_ADC_GET(par)
#define M1_MCDRV_CURR_3PH_CHAN_ASSIGN(par)
#define M1_MCDRV_CURR_3PH_CALIB_INIT(par)
#define M1_MCDRV_CURR_3PH_CALIB(par)
#define M1_MCDRV_CURR_3PH_CALIB_SET(par)

/******************************************************************************
 * Define motor 3-ph PWM control functions
 ******************************************************************************/
#define M1_MCDRV_PWM3PH_SET(par)
#define M1_MCDRV_PWM3PH_EN(par)
#define M1_MCDRV_PWM3PH_DIS(par)
#define M1_MCDRV_PWM3PH_FLT_GET(par) (1U)    

/******************************************************************************
 * Init sensors/actuators pointers
 ******************************************************************************/
#define M1_SET_PTR_DUTY(par) (g_sM1Pwm3ph.psUABC = &(par))
#define M1_SET_PTR_U_DC_BUS(par) (g_sM1AdcSensor.pf16UDcBus = &(par))
#define M1_SET_PTR_I_ABC(par) (g_sM1AdcSensor.psIABC = &(par))
#define M1_SET_PTR_SECTOR(par) (g_sM1AdcSensor.pui16SVMSector = &(par))
#define M1_SET_PTR_AUX_CHAN(par) (g_sM1AdcSensor.pui16AuxChan = &(par))

/******************************************************************************
 * global variable definitions - definitions are only template
 ******************************************************************************/
typedef struct _mcdrv_adc
{
  GMCLIB_3COOR_T_F16 *psIABC; /* pointer to the 3-phase currents */
  uint16_t *pui16SVMSector; /* pointer to the SVM sector */
  frac16_t *pui16AuxChan;   /* pointer to auxiliary ADC channel number */
  frac16_t *pf16UDcBus;     /* pointer to DC Bus voltage variable */
} mcdrv_adc_t;

typedef struct _mcdrv_pwm
{
  GMCLIB_3COOR_T_F16 *psUABC; /* pointer to the 3-phase pwm duty cycles */
} mcdrv_pwm_t;

extern mcdrv_adc_t g_sM1AdcSensor;
extern mcdrv_pwm_t g_sM1Pwm3ph;
/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* _MC_PERIPH_INIT_H_  */

/*
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PP_MEASURE_H_
#define PP_MEASURE_H_

#include "mid_def.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
extern void MID_getPp(mid_get_pp_a1_t *sPpMeasFcn);

#ifdef __cplusplus
}
#endif

#endif /* PP_MEASURE_H_ */

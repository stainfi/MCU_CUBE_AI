/**
  ******************************************************************************
  * @file    yolo_data_params.h
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-05-22T16:25:28+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#ifndef YOLO_DATA_PARAMS_H
#define YOLO_DATA_PARAMS_H

#include "ai_platform.h"

/*
#define AI_YOLO_DATA_WEIGHTS_PARAMS \
  (AI_HANDLE_PTR(&ai_yolo_data_weights_params[1]))
*/

#define AI_YOLO_DATA_CONFIG               (NULL)


#define AI_YOLO_DATA_ACTIVATIONS_SIZES \
  { 30172, }
#define AI_YOLO_DATA_ACTIVATIONS_SIZE     (30172)
#define AI_YOLO_DATA_ACTIVATIONS_COUNT    (1)
#define AI_YOLO_DATA_ACTIVATION_1_SIZE    (30172)



#define AI_YOLO_DATA_WEIGHTS_SIZES \
  { 11304, }
#define AI_YOLO_DATA_WEIGHTS_SIZE         (11304)
#define AI_YOLO_DATA_WEIGHTS_COUNT        (1)
#define AI_YOLO_DATA_WEIGHT_1_SIZE        (11304)



#define AI_YOLO_DATA_ACTIVATIONS_TABLE_GET() \
  (&g_yolo_activations_table[1])

extern ai_handle g_yolo_activations_table[1 + 2];



#define AI_YOLO_DATA_WEIGHTS_TABLE_GET() \
  (&g_yolo_weights_table[1])

extern ai_handle g_yolo_weights_table[1 + 2];


#endif    /* YOLO_DATA_PARAMS_H */

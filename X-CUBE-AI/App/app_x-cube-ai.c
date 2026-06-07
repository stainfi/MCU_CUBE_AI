
/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @author  X-CUBE-AI C code generator
  * @brief   AI program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

 /*
  * Description
  *   v1.0 - Minimum template to show how to use the Embedded Client API
  *          model. Only one input and one output is supported. All
  *          memory resources are allocated statically (AI_NETWORK_XX, defines
  *          are used).
  *          Re-target of the printf function is out-of-scope.
  *   v2.0 - add multiple IO and/or multiple heap support
  *
  *   For more information, see the embeded documentation:
  *
  *       [1] %X_CUBE_AI_DIR%/Documentation/index.html
  *
  *   X_CUBE_AI_DIR indicates the location where the X-CUBE-AI pack is installed
  *   typical : C:\Users\[user_name]\STM32Cube\Repository\STMicroelectronics\X-CUBE-AI\7.1.0
  */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#if defined ( __ICCARM__ )
#elif defined ( __CC_ARM ) || ( __GNUC__ )
#endif

/* System headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "main.h"
#include "ai_datatypes_defines.h"
#include "yolo.h"
#include "yolo_data.h"

/* USER CODE BEGIN includes */
/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/

#if !defined(AI_YOLO_INPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_in_1[AI_YOLO_IN_1_SIZE_BYTES];
ai_i8* data_ins[AI_YOLO_IN_NUM] = {
data_in_1
};
#else
ai_i8* data_ins[AI_YOLO_IN_NUM] = {
NULL
};
#endif

#if !defined(AI_YOLO_OUTPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_out_1[AI_YOLO_OUT_1_SIZE_BYTES];
ai_i8* data_outs[AI_YOLO_OUT_NUM] = {
data_out_1
};
#else
ai_i8* data_outs[AI_YOLO_OUT_NUM] = {
NULL
};
#endif

/* Activations buffers -------------------------------------------------------*/

AI_ALIGNED(32)
static uint8_t pool0[AI_YOLO_DATA_ACTIVATION_1_SIZE];

ai_handle data_activations0[] = {pool0};

/* AI objects ----------------------------------------------------------------*/

static ai_handle yolo = AI_HANDLE_NULL;

static ai_buffer* ai_input;
static ai_buffer* ai_output;

static void ai_log_err(const ai_error err, const char *fct)
{
  /* USER CODE BEGIN log */
  if (fct)
    printf("TEMPLATE - Error (%s) - type=0x%02x code=0x%02x\r\n", fct,
        err.type, err.code);
  else
    printf("TEMPLATE - Error - type=0x%02x code=0x%02x\r\n", err.type, err.code);

  do {} while (1);
  /* USER CODE END log */
}

static int ai_boostrap(ai_handle *act_addr)
{
  ai_error err;

  /* Create and initialize an instance of the model */
  err = ai_yolo_create_and_init(&yolo, act_addr, NULL);
  if (err.type != AI_ERROR_NONE) {
    ai_log_err(err, "ai_yolo_create_and_init");
    return -1;
  }

  ai_input = ai_yolo_inputs_get(yolo, NULL);
  ai_output = ai_yolo_outputs_get(yolo, NULL);

#if defined(AI_YOLO_INPUTS_IN_ACTIVATIONS)
  /*  In the case where "--allocate-inputs" option is used, memory buffer can be
   *  used from the activations buffer. This is not mandatory.
   */
  for (int idx=0; idx < AI_YOLO_IN_NUM; idx++) {
	data_ins[idx] = ai_input[idx].data;
  }
#else
  for (int idx=0; idx < AI_YOLO_IN_NUM; idx++) {
	  ai_input[idx].data = data_ins[idx];
  }
#endif

#if defined(AI_YOLO_OUTPUTS_IN_ACTIVATIONS)
  /*  In the case where "--allocate-outputs" option is used, memory buffer can be
   *  used from the activations buffer. This is no mandatory.
   */
  for (int idx=0; idx < AI_YOLO_OUT_NUM; idx++) {
	data_outs[idx] = ai_output[idx].data;
  }
#else
  for (int idx=0; idx < AI_YOLO_OUT_NUM; idx++) {
	ai_output[idx].data = data_outs[idx];
  }
#endif

  return 0;
}

static int ai_run(void)
{
  ai_i32 batch;

  batch = ai_yolo_run(yolo, ai_input, ai_output);
  if (batch != 1) {
    ai_log_err(ai_yolo_get_error(yolo),
        "ai_yolo_run");
    return -1;
  }

  return 0;
}

/* USER CODE BEGIN 2 */
#include "dcmi_ov5640.h"
#include <math.h>

/* ========== 辅助数学函数 ========== */
static float sigmoid_f(float x)
{
  return 1.0f / (1.0f + expf(-x));
}

static float expf_f(float x)
{
  return expf(x);
}

/* ========== RGB565 -> int8 转换 + 缩放到 56x56 ========== */
static void camera_to_ai_input(uint16_t *src_rgb565, ai_i8 *dst_int8,
                                uint16_t src_w, uint16_t src_h)
{
  /* 模型输入: 56x56x3, NHWC 排列, int8, scale=0.003921569 zp=-128
   * 即: int8_val = uint8_val - 128
   * 内存布局: data[y * W * C + x * C + c]
   */
  const int input_w = 56;
  const int input_h = 56;
  const int ch = 3;  /* RGB */

  for (int y = 0; y < input_h; y++) {
    /* 最近邻采样：将 240x240 映射到 56x56 */
    int src_y = (y * src_h) / input_h;
    if (src_y >= src_h) src_y = src_h - 1;

    for (int x = 0; x < input_w; x++) {
      int src_x = (x * src_w) / input_w;
      if (src_x >= src_w) src_x = src_w - 1;

      uint16_t pixel = src_rgb565[src_y * src_w + src_x];
      uint8_t r = ((pixel >> 11) & 0x1F) << 3;
      uint8_t g = ((pixel >> 5)  & 0x3F) << 2;
      uint8_t b = (pixel & 0x1F) << 3;

      /* NHWC 排列: dst[y * W * C + x * C + ch] */
      int idx = (y * input_w + x) * ch;
      dst_int8[idx + 0] = (ai_i8)((int)r - 128);
      dst_int8[idx + 1] = (ai_i8)((int)g - 128);
      dst_int8[idx + 2] = (ai_i8)((int)b - 128);
    }
  }
}

/* ========== 后处理：解析 YOLO 输出 ========== */
/* 模型输出: 1x7x7x18, int8, scale=0.142183274, zp=-15
 * 7x7 grid, 3 anchors/cell, 6 values/anchor: [cx,cy,w,h,obj_conf,cls_conf]
 */
#define YOLO_GRID       7
#define YOLO_ANCHORS    3
#define YOLO_OUT_C      18
#define YOLO_CONF_THRESH  0.5f       /* 置信度阈值 */
#define YOLO_NMS_THRESH   0.4f       /* NMS 阈值 */

typedef struct {
  float x, y, w, h;
  float conf;
} bbox_t;

bbox_t g_bboxes[20];
int    g_bbox_cnt = 0;

/* 导出版本，供 UI 访问 */
ai_bbox_t g_ai_bboxes[AI_MAX_BBOXES];
int      g_ai_bbox_cnt = 0;

static void post_process(ai_i8* data[])
{
  ai_i8 *out = data[0];
  const float scale = 0.142183274f;
  const int   zp    = -15;
  const int   img_w = 56, img_h = 56;

  g_bbox_cnt = 0;

  for (int gy = 0; gy < YOLO_GRID; gy++) {
    for (int gx = 0; gx < YOLO_GRID; gx++) {
      for (int a = 0; a < YOLO_ANCHORS; a++) {
        /* 输出排列: [grid_y, grid_x, anchor*6 + offset] */
        int base = (gy * YOLO_GRID + gx) * YOLO_OUT_C + a * 6;

        /* 反量化：real = (q - zero_point) * scale, zero_point = -15, scale = 0.1422 */
        float cx    = ((float)out[base + 0] - zp) * scale;
        float cy    = ((float)out[base + 1] - zp) * scale;
        float w     = ((float)out[base + 2] - zp) * scale;
        float h     = ((float)out[base + 3] - zp) * scale;
        float obj   = ((float)out[base + 4] - zp) * scale;
        float cls   = ((float)out[base + 5] - zp) * scale;

        /* obj/cls 原值经 sigmoid 后已是概率，再乘一次确保在 0~1 */
        float conf = sigmoid_f(obj) * sigmoid_f(cls);  /* 置信度 0~1 */

        if (conf < YOLO_CONF_THRESH) continue;
        if (g_bbox_cnt >= 20) break;

        /* sigmoid: 将 cx,cy 转换为相对网格的偏移 */
        float bx = (gx + sigmoid_f(cx)) / (float)YOLO_GRID;
        float by = (gy + sigmoid_f(cy)) / (float)YOLO_GRID;
        float bw = expf_f(w) / (float)YOLO_GRID;
        float bh = expf_f(h) / (float)YOLO_GRID;

        /* 转为像素坐标 [0, img_w/Display_Width] */
        g_bboxes[g_bbox_cnt].x    = (bx - bw * 0.5f) * (float)Display_Width;
        g_bboxes[g_bbox_cnt].y    = (by - bh * 0.5f) * (float)Display_Height;
        g_bboxes[g_bbox_cnt].w    = bw * (float)Display_Width;
        g_bboxes[g_bbox_cnt].h    = bh * (float)Display_Height;
        g_bboxes[g_bbox_cnt].conf = conf;
        g_bbox_cnt++;
      }
    }
  }

  /* NMS */
  for (int i = 0; i < g_bbox_cnt; i++) {
    if (g_bboxes[i].conf == 0) continue;
    for (int j = i + 1; j < g_bbox_cnt; j++) {
      if (g_bboxes[j].conf == 0) continue;
      /* 计算 IoU */
      float xi1 = (g_bboxes[i].x > g_bboxes[j].x) ? g_bboxes[i].x : g_bboxes[j].x;
      float yi1 = (g_bboxes[i].y > g_bboxes[j].y) ? g_bboxes[i].y : g_bboxes[j].y;
      float xi2 = (g_bboxes[i].x + g_bboxes[i].w < g_bboxes[j].x + g_bboxes[j].w)
                    ? (g_bboxes[i].x + g_bboxes[i].w) : (g_bboxes[j].x + g_bboxes[j].w);
      float yi2 = (g_bboxes[i].y + g_bboxes[i].h < g_bboxes[j].y + g_bboxes[j].h)
                    ? (g_bboxes[i].y + g_bboxes[i].h) : (g_bboxes[j].y + g_bboxes[j].h);
      float iw = xi2 - xi1;
      float ih = yi2 - yi1;
      if (iw <= 0 || ih <= 0) continue;
      float inter = iw * ih;
      float area_i = g_bboxes[i].w * g_bboxes[i].h;
      float area_j = g_bboxes[j].w * g_bboxes[j].h;
      float iou = inter / (area_i + area_j - inter);
      if (iou > YOLO_NMS_THRESH) {
        if (g_bboxes[i].conf < g_bboxes[j].conf)
          g_bboxes[i].conf = 0;  /* 抑制 i */
        else
          g_bboxes[j].conf = 0;  /* 抑制 j */
      }
    }
  }

  /* 同步到导出数组（逐字段赋值，避免类型不兼容警告） */
  g_ai_bbox_cnt = 0;
  for (int i = 0; i < g_bbox_cnt; i++) {
    if (g_bboxes[i].conf > 0 && g_ai_bbox_cnt < AI_MAX_BBOXES) {
      g_ai_bboxes[g_ai_bbox_cnt].x    = g_bboxes[i].x;
      g_ai_bboxes[g_ai_bbox_cnt].y    = g_bboxes[i].y;
      g_ai_bboxes[g_ai_bbox_cnt].w    = g_bboxes[i].w;
      g_ai_bboxes[g_ai_bbox_cnt].h    = g_bboxes[i].h;
      g_ai_bboxes[g_ai_bbox_cnt].conf = g_bboxes[i].conf;
      g_ai_bbox_cnt++;
    }
  }
}

int acquire_and_process_data(ai_i8* data[])
{
  /* 将摄像头 RGB565 图像转换为模型输入 int8 格式(使用 Process_Buffer 快照) */
  camera_to_ai_input((uint16_t*)Process_Buffer, data[0],
                      Display_Width, Display_Height);
  return 0;
}

/* 获取检测到的人脸数量 */
int AI_GetFaceCount(void)
{
  int cnt = 0;
  for (int i = 0; i < g_bbox_cnt; i++) {
    if (g_bboxes[i].conf > 0) cnt++;
  }
  return cnt;
}

/* 获取最佳人脸置信度 */
float AI_GetBestConfidence(void)
{
  float best = 0;
  for (int i = 0; i < g_bbox_cnt; i++) {
    if (g_bboxes[i].conf > best) best = g_bboxes[i].conf;
  }
  return best;
}
/* USER CODE END 2 */

/* Entry points --------------------------------------------------------------*/

void MX_X_CUBE_AI_Init(void)
{
    /* USER CODE BEGIN 5 */
  printf("\r\nTEMPLATE - initialization\r\n");

  ai_boostrap(data_activations0);
    /* USER CODE END 5 */
}

void MX_X_CUBE_AI_Process(void)
{
    /* USER CODE BEGIN 6 */
  int res = -1;

  if (yolo) {
    /* 1 - 获取并预处理输入数据（摄像头 RGB565 → 模型 int8 56x56x3） */
    res = acquire_and_process_data(data_ins);

    /* 2 - 执行 AI 推理 */
    if (res == 0)
      res = ai_run();

    /* 3 - 后处理：解析 YOLO 输出，提取人脸检测框 */
    if (res == 0)
      post_process(data_outs);
  }

  if (res) {
    ai_error err = {AI_ERROR_INVALID_STATE, AI_ERROR_CODE_NETWORK};
    ai_log_err(err, "Process has FAILED");
  }
    /* USER CODE END 6 */
}
#ifdef __cplusplus
}
#endif

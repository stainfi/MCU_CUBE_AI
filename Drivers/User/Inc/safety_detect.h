#ifndef __SAFETY_DETECT_H
#define __SAFETY_DETECT_H

#include <stdint.h>

// ================= HSV 阈值 =================
typedef struct {
    uint8_t h_min, h_max;
    uint8_t s_min, s_max;
    uint8_t v_min, v_max;
} HSV_Threshold1;

// ================= 连通域结构 =================
typedef struct {
    uint16_t min_x, max_x;
    uint16_t min_y, max_y;
    uint32_t area;
} Blob;

// ================= 外部接口 =================
void Safety_Detect_Init(void);

void Safety_Detect_Run(uint16_t* frame, uint16_t width, uint16_t height);

uint8_t Safety_Helmet_Flag(void);
uint8_t Safety_Vest_Flag(void);

#endif

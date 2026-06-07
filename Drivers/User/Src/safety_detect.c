#include "safety_detect.h"

// ================= 参数配置 =================
#define MAX_BLOBS      10
#define WIDTH_MAX      240
#define HEIGHT_MAX     240

static uint8_t mask[WIDTH_MAX * HEIGHT_MAX];



// HSV阈值（原版，已验证可用）
static HSV_Threshold1 helmet_th = {10, 45, 80, 255, 85, 255};
static HSV_Threshold1 vest_th   = {5, 35, 100, 255, 120, 255};

// 检测结果
static uint8_t helmet_flag = 0;
static uint8_t vest_flag   = 0;

// ================= RGB565 =================
#define RGB565_R(p)  ((((p)>>11)&0x1F)<<3)
#define RGB565_G(p)  ((((p)>>5)&0x3F)<<2)
#define RGB565_B(p)  (((p)&0x1F)<<3)

// ================= HSV转换 =================
static uint8_t rgb_to_h(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t maxv = (r > g) ? ((r > b)?r:b) : ((g > b)?g:b);
    uint8_t minv = (r < g) ? ((r < b)?r:b) : ((g < b)?g:b);
    if (maxv == minv) return 0;

    int16_t h;
    if (maxv == r)      h = 43 * (g - b) / (maxv - minv);
    else if (maxv == g) h = 85 + 43 * (b - r) / (maxv - minv);
    else                h = 171 + 43 * (r - g) / (maxv - minv);

    if (h < 0) h += 180;
    return h;
}

static uint8_t rgb_to_s(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t maxv = (r > g) ? ((r > b)?r:b) : ((g > b)?g:b);
    if (maxv == 0) return 0;
    uint8_t minv = (r < g) ? ((r < b)?r:b) : ((g < b)?g:b);
    return ((maxv - minv) * 255) / maxv;
}

static uint8_t rgb_to_v(uint8_t r, uint8_t g, uint8_t b)
{
    return (r > g) ? ((r > b)?r:b) : ((g > b)?g:b);
}

// ================= 生成mask =================
static void Generate_Mask(uint16_t* frame, uint16_t width,
													uint16_t height,uint8_t bif ,HSV_Threshold1* th) //bif用来开启是否对半检测，2为上半，1为下半，0为不开启
{
  uint16_t start = 0;
	uint16_t end = height;
	switch (bif) {
	case	2:  start = 0; end = height / 2; break; 
	case	1:  start = height / 2; end = height; break;
	case	0:  break;		
	}
	
	for (uint16_t y = start; y < end; y += 2)
    {
        for (uint16_t x = 0; x < width; x += 2)
        {
            uint16_t idx = y * width + x;

            uint16_t pixel = frame[idx];

            uint8_t r = RGB565_R(pixel);
            uint8_t g = RGB565_G(pixel);
            uint8_t b = RGB565_B(pixel);

            uint8_t h = rgb_to_h(r,g,b);
            uint8_t s = rgb_to_s(r,g,b);
            uint8_t v = rgb_to_v(r,g,b);

            if (h>=th->h_min && h<=th->h_max &&
                s>=th->s_min && s<=th->s_max &&
                v>=th->v_min && v<=th->v_max)
            {
                mask[idx] = 1;
            }
            else
            {
                mask[idx] = 0;
            }
        }
    }
}


void Safety_Detect_Run(uint16_t* frame, uint16_t width, uint16_t height)
{
    helmet_flag = 0;
    vest_flag = 0;

    uint32_t count = 0;
    uint32_t total = 0;

    // ===== 安全帽（上半区）=====
    Generate_Mask(frame, width, height,2, &helmet_th);

    count = 0;
    total = height * width /2;

    for (uint16_t y = 0; y < height / 2; y += 2)
    {
        for (uint16_t x = 0; x < width; x += 2)
        {
            uint32_t idx = y * width + x;

            if (mask[idx])
                count++;

        }
    }

    float ratio = (float)count / total;

    if (ratio > 0.05f && ratio < 0.20f)
        helmet_flag = 1;


    // ===== 反光衣（下半区）=====
    Generate_Mask(frame, width, height,1, &vest_th);

    count = 0;

    for (uint16_t y = height / 2; y < height; y += 2)
    {
        for (uint16_t x = 0; x < width; x += 2)
        {
            uint32_t idx = y * width + x;

            if (mask[idx])
                count++;

        }
    }

    float ratio2 = (float)count / total;

    if (ratio2 > 0.25f)
        vest_flag = 1;
}

// ================= 接口 =================
uint8_t Safety_Helmet_Flag(void)
{
    return helmet_flag;
}

uint8_t Safety_Vest_Flag(void)
{
    return vest_flag;
}

void Safety_Detect_Init(void)
{
    helmet_flag = 0;
    vest_flag = 0;
}

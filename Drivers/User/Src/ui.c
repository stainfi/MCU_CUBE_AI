#include "ui.h"
#include "main.h"

#include <stdio.h>
#include "lcd_spi_154.h"
#include "dcmi_ov5640.h"
#include "app_x-cube-ai.h"

static uint16_t ui_dirty = 1;
static char ui_face[20];      /* 人脸检测 */
static char ui_fps[20];       /* 帧率 */
static char ui_conf[20];      /* 置信度 */

/* 画面只显示在中间区域 */
#define CAM_X       0
#define CAM_Y       30
#define CAM_W       240
#define CAM_H       160


void UI_UpdateData(void)
{
    /* 人脸检测 */
    sprintf(ui_face, "Face:%d", (int)face_detect_count);

    /* FPS + 置信度 */
    sprintf(ui_fps, "FPS:%d", OV5640_FPS);
    sprintf(ui_conf, "%.2f", AI_GetBestConfidence());

    ui_dirty = 1;
}

void UI_Draw(void)
{
    if (!ui_dirty) return;
    ui_dirty = 0;

    LCD_Clear();
    LCD_CopyBuffer(CAM_X, CAM_Y, CAM_W, CAM_H, (uint16_t*)Process_Buffer);

    /* 绘制 AI 人脸检测框 */
    {
      extern ai_bbox_t g_ai_bboxes[];
      extern int      g_ai_bbox_cnt;
      LCD_SetColor(LCD_RED);   /* 红色画框 */
      for (int i = 0; i < g_ai_bbox_cnt; i++) {
        int bx = (int)g_ai_bboxes[i].x;
        int by = (int)g_ai_bboxes[i].y + CAM_Y;
        int bw = (int)g_ai_bboxes[i].w;
        int bh = (int)g_ai_bboxes[i].h;
        if (bx < 0) { bw += bx; bx = 0; }
        if (by < CAM_Y) { bh -= (CAM_Y - by); by = CAM_Y; }
        if (bx + bw > LCD_Width)  bw = LCD_Width - bx;
        if (by + bh > LCD_Height) bh = LCD_Height - by;
        if (bw > 2 && bh > 2) {
          LCD_DrawRect(bx, by, bw, bh);
        }
      }
    }

    /* 56×56 参考框（屏幕中心，辅助对焦/摄像头校准，注释掉可隐藏） */
    {
      int ref_w = 56, ref_h = 56;
      int ref_x = (LCD_Width  - ref_w) / 2;
      int ref_y = (LCD_Height - ref_h) / 2;
      LCD_SetColor(LCD_GREEN);
      LCD_DrawRect(ref_x, ref_y, ref_w, ref_h);
    }

    /* 左上：人脸数 + 置信度 */
    LCD_DisplayString(1, 5, ui_face);
    LCD_DisplayString(100, 5, ui_conf);

    /* 右上：FPS */
    LCD_DisplayString(165, 5, ui_fps);

}

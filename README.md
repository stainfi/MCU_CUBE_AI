# STM32H750 + OV5640 AI 人脸检测系统

基于 **鹿小班 STM32H750 开发板** 与 **OV5640 摄像头** 的实时 AI 人脸检测项目，使用 STM32CubeMX + X-CUBE-AI 工具链，在 Keil MDK-ARM 环境下开发。

---

## 硬件平台

| 组件 | 型号/参数 |
|------|----------|
| 主控 MCU | STM32H750VBTx (Cortex-M7, 480MHz) |
| 开发板 | 鹿小班 STM32H750 开发板 |
| 摄像头 | OV5640 (DCMI 接口, 自动对焦) |
| 显示屏 | SPI LCD (240×240) |
| 调试接口 | SWD / USART1 |

## 软件框架

```
├── Core/                       # STM32CubeMX 生成的核心代码
│   ├── Inc/                    # 头文件 (main.h, stm32h7xx_it.h 等)
│   └── Src/                    # 源文件 (main.c, HAL MSP, 中断服务 等)
├── Drivers/                    # HAL 驱动库
│   ├── CMSIS/                  # Cortex-M CMSIS
│   └── STM32H7xx_HAL_Driver/   # STM32H7 HAL 驱动
├── Middlewares/ST/             # ST 中间件
│   └── AI/                     # X-CUBE-AI 运行时库
├── X-CUBE-AI/                  # AI 模型与应用
│   ├── App/                    # 模型推理 & 后处理
│   │   ├── app_x-cube-ai.c/h   # AI 初始化与推理入口
│   │   ├── yolo.c/h            # YOLO 网络模型 (自动生成)
│   │   ├── yolo_data.c/h       # 网络权重数据
│   │   └── yolo_config.h       # 网络配置
│   └── constants_ai.h          # AI 常量定义
├── User/                       # 用户外设驱动
│   ├── Inc/                    # LED, LCD, DCMI, USART, UI 等头文件
│   └── Src/                    # 对应源文件
└── MDK-ARM/                    # Keil MDK 工程文件
    ├── STM32H750.uvprojx       # 工程文件
    ├── STM32H750.sct           # 链接脚本
    └── startup_stm32h750xx.s   # 启动文件
```

## AI 模型

| 属性 | 值 |
|------|-----|
| 模型名称 | `yoloface_int8` |
| 框架 | TensorFlow Lite → X-CUBE-AI |
| 量化方式 | INT8 量化 (QLinear) |
| 输入尺寸 | 56×56×3 (9.19 KB) |
| 输出尺寸 | 7×7×18 (882 Bytes) |
| 计算量 | ~1.38M MACC |
| 权重大小 | 11.04 KiB |
| 运行时 RAM | 29.46 KiB |
| 转换工具 | ST Edge AI Core v2.2.0 |

## 功能说明

1. **OV5640 DCMI 摄像头采集** — 通过 DCMI 接口 + DMA 连续采集 240×240 图像
2. **自动对焦** — 上电后下载 AF 固件，采用持续自动对焦模式
3. **帧处理流水线** — 双缓冲区设计（Camera_Buffer → Process_Buffer），采集与 AI 推理并行
4. **AI 人脸检测** — YOLO 轻量级人脸检测网络，INT8 量化推理
5. **LCD 实时显示** — 在 SPI LCD 上绘制摄像头画面与检测框
6. **串口调试输出** — USART1 输出检测信息

## 内存布局

```
Camera_Buffer  @ 0x24000000   (DCMI DMA 写入)
Process_Buffer @ 0x24020000   (AI 处理缓冲区)
帧大小: 240 × 240 × 2 = 115,200 Bytes (RGB565)
```

## 开发环境

| 工具 | 版本 |
|------|------|
| IDE | Keil MDK-ARM 5 |
| STM32CubeMX | 生成初始化代码 |
| X-CUBE-AI | AI 模型部署 |
| ST Edge AI Core | v2.2.0 (模型转换) |
| 编译器 | ARM Compiler 5/6 |

## 快速开始

1. 使用 Keil MDK-ARM 打开 `MDK-ARM/STM32H750.uvprojx`
2. 确认工程配置中的芯片型号为 `STM32H750VBTx`
3. 编译工程（Rebuild all）
4. 通过 SWD/J-Link/ST-Link 下载到开发板
5. 复位后系统自动开始人脸检测，LCD 实时显示画面与检测结果

## 目录说明

- `Core/` — STM32CubeMX 自动生成，包含 `main.c` 主逻辑
- `Drivers/` — STM32H7 HAL 库 & CMSIS
- `Middlewares/ST/AI/` — X-CUBE-AI 运行时
- `X-CUBE-AI/App/` — YOLO 模型文件与应用层封装
- `X-CUBE-AI/constants_ai.h` — AI 相关常量
- `User/` — 外设驱动（LED, LCD, OV5640, USART, UI）
- `MDK-ARM/` — Keil 工程与编译输出

## 注意事项

- 需确保 `MPU_Config()` 正确配置了 DCMI DMA 缓冲区为不可缓存/写透模式，保证数据一致性
- CPU Cache (I-Cache / D-Cache) 已使能，帧处理前调用 `SCB_InvalidateDCache_by_Addr` 刷新缓存
- OV5640 自动对焦固件需在初始化时下载 (`OV5640_AF_Download_Firmware`)
- 若使用广角镜头（120°/160°），可能需要调整翻转/镜像设置
- 模型文件 (`yolo.c/h`, `yolo_data.c/h`) 由 ST Edge AI Core 自动生成，请勿手动修改


#ifndef GPU_H
#define GPU_H

#include <3ds/types.h>

#define GPU_GP0  (0x1F801810)
#define GPU_GP1  (0x1F801814)
#define GPU_READ (0x1F801810)
#define GPU_STAT (0x1F801814)

#define GP0_COMMAND_NOP         (0x00)
#define GP0_COMMAND_CLEAR_CACHE (0x01)
#define GP0_COMMAND_FILL_RECT   (0x02)
#define GP0_COMMAND_VRAM_VRAM   (0x80)
#define GP0_COMMAND_CPU_VRAM    (0xA0)
#define GP0_COMMAND_VRAM_CPU    (0xC0)
#define GP0_COMMAND_IRQ1        (0x1F)

#define GP1_COMMAND_RST         (0x00)
#define GP1_COMMAND_RST_FIFO    (0x01)
#define GP1_COMMAND_ACK_IRQ1    (0x02)
#define GP1_COMMAND_DISP_EN     (0x03)
#define GP1_COMMAND_DMA_DIR     (0x04)
#define GP1_COMMAND_DISP_AREA   (0x05)
#define GP1_COMMAND_HORIZ_RANGE (0x06)
#define GP1_COMMAND_VERT_RANGE  (0x07)
#define GP1_COMMAND_DISP_MODE   (0x08)
#define GP1_COMMAND_TEX_DIS     (0x09)
#define GP1_COMMAND_GPU_INFO    (0x10)
#define GP1_COMMAND_OLD_TEX_DIS (0x20)

struct GPU
{
    u32 status;
};

#endif

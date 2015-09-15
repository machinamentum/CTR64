/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef GPU_H
#define GPU_H

#include "platform.h"
#include "mips.h"

#ifdef _3DS
#include <GL/gl.h>
#else
#include <GLFW/glfw3.h>
#endif

#define GPU_GP0  (0x1F801810)
#define GPU_GP1  (0x1F801814)
#define GPU_READ (0x1F801810)
#define GPU_STAT (0x1F801814)

#define GPU_STAT_DISP_EN        (1 << 23)

#define GP0_COMMAND_NOP         (0x00)
#define GP0_COMMAND_CLEAR_CACHE (0x01)
#define GP0_COMMAND_FILL_RECT   (0x02)
#define GP0_COMMAND_VRAM_VRAM   (0x80)
#define GP0_COMMAND_CPU_VRAM    (0xA0)
#define GP0_COMMAND_VRAM_CPU    (0xC0)
#define GP0_COMMAND_IRQ1        (0x1F)
#define GP0_COMMAND_DRAW_MODE   (0xE1)

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

#define GTE_FUNC_SF             (1 << 19)
#define GTE_FUNC_MULT_MTX       (3 << 17)
#define GTE_FUNC_MULT_VEC       (3 << 15)
#define GTE_FUNC_TRNS_VEC       (3 << 13)
#define GTE_FUNC_SATURATE       (1 << 10)
#define GTE_FUNC_CMD            (0b111111)

#define GTE_REG_VXY0            (0)
#define GTE_REG_VZ0             (1)
#define GTE_REG_VXY1            (2)
#define GTE_REG_VZ1             (3)
#define GTE_REG_VXY2            (4)
#define GTE_REG_VZ2             (5)
#define GTE_REG_RGBC            (6)
#define GTE_REG_OTZ             (7)
#define GTE_REG_IR0             (8)
#define GTE_REG_IR1             (9)
#define GTE_REG_IR2            (10)
#define GTE_REG_IR3            (11)
#define GTE_REG_SXY0           (12)
#define GTE_REG_SXY1           (13)
#define GTE_REG_SXY2           (14)
#define GTE_REG_SXYP           (15)
#define GTE_REG_SZ0            (16)
#define GTE_REG_SZ1            (17)
#define GTE_REG_SZ2            (18)
#define GTE_REG_SZ3            (19)
#define GTE_REG_RGB0           (20)
#define GTE_REG_RGB1           (21)
#define GTE_REG_RGB2           (22)
#define GTE_REG_RES1           (23)
#define GTE_REG_MAC0           (24)
#define GTE_REG_MAC1           (25)
#define GTE_REG_MAC2           (26)
#define GTE_REG_MAC3           (27)
#define GTE_REG_IRGB           (28)
#define GTE_REG_ORGB           (29)
#define GTE_REG_LZCS           (30)
#define GTE_REG_LZCR           (31)

#define GTE_REG_ROT_MTX0       (32)
#define GTE_REG_ROT_MTX1       (33)
#define GTE_REG_ROT_MTX2       (34)
#define GTE_REG_ROT_MTX3       (35)
#define GTE_REG_ROT_MTX4       (36)
#define GTE_REG_TRX            (37)
#define GTE_REG_TRY            (38)
#define GTE_REG_TRZ            (39)
#define GTE_REG_LIG_SRC_MTX0   (40)
#define GTE_REG_LIG_SRC_MTX1   (41)
#define GTE_REG_LIG_SRC_MTX2   (42)
#define GTE_REG_LIG_SRC_MTX3   (43)
#define GTE_REG_LIG_SRC_MTX4   (44)
#define GTE_REG_RBK            (45)
#define GTE_REG_BGK            (46)
#define GTE_REG_BBK            (47)
#define GTE_REG_LIG_CLR_MTX0   (48)
#define GTE_REG_LIG_CLR_MTX1   (49)
#define GTE_REG_LIG_CLR_MTX2   (50)
#define GTE_REG_LIG_CLR_MTX3   (51)
#define GTE_REG_LIG_CLR_MTX4   (52)
#define GTE_REG_RFC            (53)
#define GTE_REG_GFC            (54)
#define GTE_REG_BFC            (55)
#define GTE_REG_OFX            (56)
#define GTE_REG_OFY            (57)
#define GTE_REG_H              (58)
#define GTE_REG_DQA            (59)
#define GTE_REG_DQB            (60)
#define GTE_REG_ZSF3           (61)
#define GTE_REG_ZSF4           (62)
#define GTE_REG_FLAG           (63)

#define GPU_VRAM_LINES         (512)
#define GPU_VRAM_LINE_SIZE     (1024)

#define GPU_TEXTURE_COLOR_4BIT     (0)
#define GPU_TEXTURE_COLOR_8BIT     (1)
#define GPU_TEXTURE_COLOR_15BIT    (2)
#define GPU_TEXTURE_COLOR_RESERVED (3)

struct GPU : public Coprocessor
{

    GPU();
    void *GfxHandle;
    u32 Status;
    u32 Gp0PacketsLeft = 0;
    u32 Gp0WaitingCmd;
    u32 Gp0Packets[16];
    GLuint TempTex;
    u32 VRT = 0;
    u32 *VRAM = (u32 *)linearAlloc(GPU_VRAM_LINES * GPU_VRAM_LINE_SIZE * 2);
    void (*Gp0Func)(GPU *, u32) = NULL;

    u32 DrawMode;
    struct
    {
        u32 X1;
        u32 Y1;
        u32 X2;
        u32 Y2;
    } DrawAreaBounds;
};

void GpuGp0(void *, u32);
void GpuGp1(void *, u32);
void DMA2Trigger(void *, u32);
u32  GpuStat(void *, u32);

#endif

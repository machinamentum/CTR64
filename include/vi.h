/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef VI_H
#define VI_H

#include "mips.h"

#define VI_COLOR_DEPTH(x)       (x & 0b0011)
#define VI_GAMMA_DITHER         (1 << 2)
#define VI_GAMMA_BOOST          (1 << 3)
#define VI_DIVOT                (1 << 4)
#define VI_SERRATE              (1 << 6)
#define VI_ANTI_ALIAS_MODE(x)   ((x & (3 << 8)) >> 8)
#define VI_PIXEL_ADVANCE(x)     ((x & (0b1111 << 12)) >> 12)
#define VI_DITHER_FILTER        (1 << 16)

#define VI_HSYNC_WIDTH(x)       (x & 0xFF)
#define VI_BURST_WIDTH(x)       ((x & 0xFF00) >> 8)
#define VI_VSYNC_WIDTH(x)       ((x & (0b1111 << 16)) >> 16)
#define VI_BURST_START(x)       ((x & (0x3FF << 20) >> 20)

#define VI_LINE_DURATION(x)     (x & 0xFFF)
#define VI_HSYNC_PERIOD(x)      (x & (0x1F << 16) >> 16)

#define VI_LINE_DURATION_HI(x)  (x & (0xFFF << 16) >> 16)

#define VI_H_END(x)             (x & 0x3FF)
#define VI_H_START(x)           ((x & (0x3FF << 16) >> 16)

#define VI_V_END(x)             (x & 0x3FF)
#define VI_V_START(x)           ((x & (0x3FF << 16) >> 16)

#define VI_COLOR_BURST_END(x)   (x & 0x3FF)
#define VI_COLOR_BURST_START(x) ((x & (0x3FF << 16) >> 16)

// Note: fixed point 2.10
#define VI_H_SCALE(x)           (x & 0xFFF)
#define VI_H_SUBPIXEL_OFFSET(x) (x & (0xFFF << 16) >> 16)

#define VI_V_SCALE(x)           (x & 0xFFF)
#define VI_V_SUBPIXEL_OFFSET(x) (x & (0xFFF << 16) >> 16)

struct VideoInterface
{
    u32 Control;
    u32 DRAMAddr;
    u32 HWidth;
    u32 VInterrupt;
    u32 VCurrentLine;
    u32 Timing;
    u32 VSync;
    u32 HSync;
    u32 HSyncLeap;
    u32 HVideo;
    u32 VVideo;
    u32 VBurst;
    u32 XScale;
    u32 YScale;
};

void VIStartThread(MIPS_R3000 *Cpu, VideoInterface *VI);
void VICloseThread();

#endif

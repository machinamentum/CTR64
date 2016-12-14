/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef PI_H
#define PI_H

#include "platform.h"
#include "mips.h"

#define PI_ADDRESS24_BITS   (0xFFFFFF)
#define PI_LATENCY_BITS     (0xFF)
#define PI_PAGE_SIZE_BITS   (0xF)
#define PI_DOM_RELEASE_BIT  (1 << 0)

#define PI_STATUS_DMA_BUSY  (1 << 0)
#define PI_STATUS_IO_BUSY   (1 << 1)
#define PI_STATUS_ERROR     (1 << 2)

struct PACKED PeripheralInterface
{
    u32 DRAMAddr;
    u32 CartAddr;
    u32 ReadLen;
    u32 WriteLen;
    u32 Status;
    u32 DOM1Latency;
    u32 DOM1PulseWidth;
    u32 DOM1PageSize;
    u32 DOM1Release;
    u32 DOM2Latency;
    u32 DOM2PulseWidth;
    u32 DOM2PageSize;
    u32 DOM2Release;
};

void PIStartThread(MIPS_R3000 *C, PeripheralInterface *PI);
void PICloseThread();

#endif

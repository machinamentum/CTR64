/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef SP_H
#define SP_H

#include "platform.h"

#define SP_STATUS_HALT (1 << 0)

struct PACKED SignalProcessor
{
    u32 MemAddr;
    u32 DRAMAddr;
    u32 ReadDMALength;
    u32 WriteDMALength;
    u32 Status = __builtin_bswap32(SP_STATUS_HALT);
    u32 DMAFull;
    u32 DMABusy;
    u32 Semaphore;
};


#endif

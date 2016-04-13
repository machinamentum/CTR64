/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "pi.h"

static MIPS_R3000 *Cpu;
static PeripheralInterface *PI;
static bool KeepThreadActive;
static void *ThreadHandle;

static void
PIUpdateRoutine()
{
    while (KeepThreadActive)
    {
        if (PI->Status & __builtin_bswap32(0x2)) PI->Status &= ~ __builtin_bswap32(0x2);
        if (PI->Status & __builtin_bswap32(0x1)) PI->Status &= ~ __builtin_bswap32(0x1);
        u32 CartToRAMLen = __builtin_bswap32(PI->WriteLen);
        if (CartToRAMLen)
        {
            PI->Status |= __builtin_bswap32(PI_STATUS_DMA_BUSY);
            u32 RAMAddr = __builtin_bswap32(PI->DRAMAddr) & 0x00FFFFFF;
            u32 CartAddr = __builtin_bswap32(PI->CartAddr) & 0x1FFFFFFF;
            for (u32 i = 0; i < CartToRAMLen; i += 4)
            {
                u32 Value = ReadMemWordRaw(Cpu, CartAddr + i);
                WriteMemWordRaw(Cpu, RAMAddr + i, Value);
            }
            PI->WriteLen = 0;
            PI->Status &= ~__builtin_bswap32(PI_STATUS_DMA_BUSY);
        }

        PlatformSleepThread(PLATFORM_SLEEP_SECONDS(1));
    }
}


void
PIStartThread(MIPS_R3000 *C, PeripheralInterface *P)
{
    PI = P;
    Cpu = C;
    KeepThreadActive = true;
    ThreadHandle = PlatformCreateThread(PIUpdateRoutine);
}

void
PICloseThread()
{
    KeepThreadActive = false;
    PlatformJoinThread(ThreadHandle);
}

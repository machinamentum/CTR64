/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "pi.h"


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
    }
}


void
PIStartThread(PeripheralInterface *P)
{
    PI = P;
    KeepThreadActive = true;
    ThreadHandle = PlatformCreateThread(PIUpdateRoutine);
}

void
PICloseThread()
{
    KeepThreadActive = false;
    PlatformJoinThread(ThreadHandle);
}

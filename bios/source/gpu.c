/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

unsigned int * const GP0 = (unsigned int *)0x1F801810;
unsigned int * const GP1 = (unsigned int *)0x1F801814;
//static const unsigned int *GPUREAD = (unsigned int *)0x1F801810;
static const unsigned int *GPUSTAT = (unsigned int *)0x1F801814;

int
gpu_sync()
{
    printf(__FUNCTION__);
    // TODO gpu_sync
    return -1;
}

int
GPU_cw(unsigned int Cmd)
{
    printf(__FUNCTION__);
    int Result = gpu_sync();
    *GP0 = Cmd;
    return Result;
}

int GPU_cwp(void *Src, unsigned int Num)
{
    printf(__FUNCTION__);
    gpu_sync();
    unsigned int *Words = (unsigned int *)Src;
    for (unsigned int i = 0; i < Num; ++i)
    {
        *GP0 = Words[i];
    }
    return 0;
}

void
SendGP1Command(unsigned int Cmd)
{
    printf(__FUNCTION__);
    *GP1 = Cmd;
}

unsigned int
GetGPUStatus()
{
    printf(__FUNCTION__);
    return *GPUSTAT;
}

void *
gpu_send_dma(int Xdst, int Ydst, int Xsiz, int Ysiz, void *Src)
{
    printf(__FUNCTION__);
    return GP0;
}

void
send_gpu_linked_list(void *Src)
{
    printf(__FUNCTION__);
}

void *
gpu_abort_dma()
{
    printf(__FUNCTION__);
    return GP1;
}

void
GPU_dw(int Xdst, int Ydst, int Xsiz, int Ysiz, void *Src)
{
    printf(__FUNCTION__);
    // TODO GPU_dw
}

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
    // TODO gpu_sync
    return -1;
}

int
GPU_cw(unsigned int Cmd)
{
    int Result = gpu_sync();
    *GP0 = Cmd;
    return Result;
}

int GPU_cwp(void *Src, unsigned int Num)
{
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
    *GP1 = Cmd;
}

unsigned int
GetGPUStatus()
{
    return *GPUSTAT;
}

void *
gpu_send_dma(int Xdst, int Ydst, int Xsiz, int Ysiz, void *Src)
{
    return GP0;
}

void
send_gpu_linked_list(void *Src)
{

}

void *
gpu_abort_dma()
{
    return GP1;
}

void
GPU_dw(int Xdst, int Ydst, int Xsiz, int Ysiz, void *Src)
{
    // TODO GPU_dw
}
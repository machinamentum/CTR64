/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

int
CdGetLbn(const char *FileName)
{
    printf(__FUNCTION__);
    return -1;
}

int
CdReadSector()
{
    printf(__FUNCTION__);
    return -1;
}

int
CdGetStatus()
{
    printf(__FUNCTION__);
    return -1;
}

int
CdAsyncSeekL(void *Src)
{
    printf(__FUNCTION__);
    return 0;
}

int
CdAsyncGetStatus(char *Dst)
{
    printf(__FUNCTION__);
    return 0;
}

int
CdAsyncReadSector(int Count, void *Dst, int Mode)
{
    printf(__FUNCTION__);
    return 0;
}

void
CdAsyncSetMode(int Mode)
{
    printf(__FUNCTION__);
}

void
CdromGetInt5errCode(void *Dst1, void *Dst2)
{
    printf(__FUNCTION__);
}

void
CdInitSubFunc()
{
    printf(__FUNCTION__);
}

void
CdInit()
{
    printf(__FUNCTION__);
}

void
CdRemove()
{
    printf(__FUNCTION__);
}

void
CdromIoIrqFunc1()
{
    printf(__FUNCTION__);
}

void
CdromDmaIrqFunc1()
{
    printf(__FUNCTION__);
}

void
CdromIoIrqFunc2()
{
    printf(__FUNCTION__);
}

void
CdromDmaIrqFunc2()
{
    printf(__FUNCTION__);
}

void
SetCdromIrqAutoAbort(int Type, int Flag)
{
    printf(__FUNCTION__);
}

void
EnqueueCdIntr()
{
    printf(__FUNCTION__);
}

void
DequeueCdIntr()
{
    printf(__FUNCTION__);
}

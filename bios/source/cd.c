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
    return -1;
}

int
CdReadSector()
{
    return -1;
}

int
CdGetStatus()
{
    return -1;
}

int
CdAsyncSeekL(void *Src)
{
    return 0;
}

int
CdAsyncGetStatus(char *Dst)
{
    return 0;
}

int
CdAsyncReadSector(int Count, void *Dst, int Mode)
{
    return 0;
}

void
CdAsyncSetMode(int Mode)
{

}

void
CdromGetInt5errCode(void *Dst1, void *Dst2)
{
    
}

void
CdInitSubFunc()
{

}

void
CdInit()
{

}

void
CdRemove()
{

}

void
CdromIoIrqFunc1()
{

}

void
CdromDmaIrqFunc1()
{

}

void
CdromIoIrqFunc2()
{

}

void
CdromDmaIrqFunc2()
{
    
}

void
SetCdromIrqAutoAbort(int Type, int Flag)
{

}

void
EnqueueCdIntr()
{

}

void
DequeueCdIntr()
{

}

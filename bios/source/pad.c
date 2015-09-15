/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

void
InitPad(void *Buf1, int Siz1, void *Buf2, int Siz2)
{
    printf("%s\n", __FUNCTION__);
}

void
StartPad()
{
    printf("%s\n", __FUNCTION__);
}

void
StopPad()
{
    printf("%s\n", __FUNCTION__);
}

void
OutdatedPadInitAndStart(int Type, void *ButtonDst, int Unk2, int Unk3)
{
    printf("%s\n", __FUNCTION__);
}

void
OutdatedPadGetButtons()
{
    printf("%s\n", __FUNCTION__);
}

void
ChangeClearPad(int Clear)
{
    printf("%s\n", __FUNCTION__);
}
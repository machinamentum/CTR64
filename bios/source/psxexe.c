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
LoadExeHeader(const char *FileName, void *HdrBuf)
{
    printf(__FUNCTION__);
    int File = FileOpen(FileName, FILE_READ);
    if (File == -1)
    {
        printf("Could not open file: ");
        printf(FileName);
        printf("\n");
        return;
    }

    char *Hdr = malloc(0x800);
    FileRead(File, Hdr, 0x800);
    FileClose(File);
    memcpy(HdrBuf, Hdr + 0x10, 0x40);
    free(Hdr);
}

void
LoadExeFile(const char *FileName, void *HdrBuf)
{
    printf(__FUNCTION__);
}

void
DoExecute(void *HdrBuf, unsigned int unk1, unsigned int unk2)
{
    printf(__FUNCTION__);
}

void
LoadAndExecute(const char *FileName, unsigned int StackBase, unsigned int StackOffset)
{
    printf(__FUNCTION__);
}
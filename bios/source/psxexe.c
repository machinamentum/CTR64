/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

typedef struct
{
    char AsciiID[8];
    char pad[8];
    unsigned int InitPC;
    unsigned int InitGP;
    unsigned int DestAddress;
    unsigned int FileSize;
    unsigned int Unk0;
    unsigned int Unk1;
    unsigned int MemfillStartAddress;
    unsigned int MemfillSize;
    unsigned int InitSP;
    unsigned int OffsetSP;
    unsigned char Reserved[14];
    char AsciiMarker[1]; //read off the bottom of the struct
} psxexe_hdr;


void
LoadExeHeader(const char *FileName, void *HdrBuf)
{
    printf("%s\n", __FUNCTION__);
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
    printf("%s\n", __FUNCTION__);
}

void
DoExecute(void *HdrBuf, unsigned int unk1, unsigned int unk2)
{
    printf("%s\n", __FUNCTION__);
}

void
LoadAndExecute(const char *FileName, unsigned int StackBase, unsigned int StackOffset)
{
    printf("%s\n", __FUNCTION__);
    int Fd = FileOpen(FileName, FILE_READ);
    psxexe_hdr *Hdr = malloc(0x800);
    FileRead(Fd, Hdr, 0x800);
    FileRead(Fd, (void *)Hdr->DestAddress, Hdr->FileSize);
    typedef void (*UEFunc)(void);
    UEFunc UserEntry = (UEFunc)Hdr->InitPC;
    UserEntry();
}

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef PSX_EXE_H
#define PSX_EXE_H

#include "mips.h"

struct psxexe_hdr
{
    char AsciiID[8];
    u8 pad[8];
    u32 InitPC;
    u32 InitGP;
    u32 DestAddress;
    u32 FileSize;
    u32 Unk0;
    u32 Unk1;
    u32 MemfillStartAddress;
    u32 MemfillSize;
    u32 InitSP;
    u32 OffsetSP;
    u8 Reserved[14];
    char AsciiMarker[1]; //read off the bottom of the struct
};


void LoadPsxExe(MIPS_R3000 *Cpu, psxexe_hdr *Hdr);

#endif

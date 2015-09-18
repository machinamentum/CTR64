/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "psxexe.h"

void
LoadPsxExe(MIPS_R3000 *Cpu, psxexe_hdr *Hdr)
{
    u32 Address = Hdr->DestAddress;
    u32 PC = Hdr->InitPC;
    u32 GP = Hdr->InitGP;
    u32 SP = Hdr->InitSP + Hdr->OffsetSP;
    u8 *CodeStart = ((u8 *)Hdr) + 0x800;
    for (u32 i = 0; i < Hdr->FileSize; ++i)
    {
        WriteMemByteRaw(Cpu, Address + i, CodeStart[i]);
    }
    Cpu->pc = PC;
    Cpu->gp = GP;
    Cpu->sp = SP;
    Cpu->fp = SP;
}

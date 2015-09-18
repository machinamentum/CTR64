
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

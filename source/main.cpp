/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "platform.h"
#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include "mips.h"
#include "disasm.h"
#include "joypad.h"

void
ResetCpu(MIPS_R3000 *Cpu)
{
    Cpu->pc = RESET_VECTOR;
}

static u32 InterruptMask;

int main(int argc, char **argv)
{
    InitPlatform(argc, argv);

    MIPS_R3000 Cpu;

    void *RDRAM = linearAlloc(0x400000);
    MapMemoryRegion(&Cpu, (mmm) {RDRAM, 0x00000000, 0x400000});

    FILE *f = fopen("n64_ipl.bin", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *BiosBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(BiosBuffer, fsize, 1, f);
    fclose(f);

    for (int i = 0; i < fsize; ++i)
    {
        WriteMemByteRaw(&Cpu, RESET_VECTOR + i, BiosBuffer[i]);
    }
    linearFree(BiosBuffer);

    ResetCpu(&Cpu);

    bool Step = false;
    int CyclesToRun = 10000;
    bool EnableDisassembler = false;
    bool AutoStep = true;
    u32 IRQ0Steps = 0;
    while (MainLoopPlatform())
    {
#ifdef _3DS
        u32 KeysDown = hidKeysDown();

        if (KeysDown & KEY_START)
            break;
#endif

        if (Step || AutoStep)
        {
            StepCpu(&Cpu, CyclesToRun);
            IRQ0Steps += CyclesToRun;
            if (IRQ0Steps >= 50000)
            {
                C0GenerateException(&Cpu, C0_CAUSE_INT, Cpu.pc - 4);
                IRQ0Steps = 0;
                InterruptMask |= 1;
            }
        }

        if (EnableDisassembler)
        {
            printf("\x1b[0;0H");
            DisassemblerPrintRange(&Cpu, Cpu.pc - (13 * 4), 29, Cpu.pc);
        }

        SwapBuffersPlatform();
    }
    ExitPlatform();

    return 0;
}




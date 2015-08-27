/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <3ds.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include "mips.h"
#include "gpu.h"
#include "disasm.h"
#include "psxexe.h"
#include <gfx_device.h>

static void
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

void
ResetCpu(MIPS_R3000 *Cpu)
{
    Cpu->pc = RESET_VECTOR;
}

int main(int argc, char **argv)
{

    if (argc) chdir(argv[0]);

    gfxInitDefault();
    hidInit(NULL);
    PrintConsole BottomConsole;
    consoleInit(GFX_BOTTOM, &BottomConsole);

    MIPS_R3000 Cpu;
    GPU Gpu;
    Cpu.CP1 = &Gpu;
    MapRegister(&Cpu, (mmr) {GPU_GP0, &Gpu, GpuGp0});
    MapRegister(&Cpu, (mmr) {GPU_GP1, &Gpu, GpuGp1});

    FILE *f = fopen("boot.exe", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *ExeBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(ExeBuffer, fsize, 1, f);
    fclose(f);

    LoadPsxExe(&Cpu, (psxexe_hdr *)ExeBuffer);

    f = fopen("psx_bios.bin", "rb");
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *BiosBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(BiosBuffer, fsize, 1, f);
    fclose(f);

    for (int i = 0; i < fsize; ++i)
    {
        WriteMemByteRaw(&Cpu, RESET_VECTOR + i, BiosBuffer[i]);
    }

    ResetCpu(&Cpu);

    bool Step = false;

//    PrintConsole TopConsole;
//    consoleInit(GFX_TOP, &TopConsole);
//    consoleSelect(&BottomConsole);
    int CyclesToRun = 1;

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

        if (keysDown() & KEY_DDOWN)
        {
//            for (int i = 3; i >= 0; --i)
//            {
//                Stages[i] = -(i + 1);
//            }
            ResetCpu(&Cpu);
        }

        if (keysHeld() & KEY_DLEFT)
        {
            CyclesToRun -= 1000;
            if (CyclesToRun < 1)
                CyclesToRun = 1;
        }

        if (keysHeld() & KEY_DRIGHT)
        {
            CyclesToRun += 1000;
        }

        if (keysDown() & KEY_DUP)
        {
            CyclesToRun = 1;
        }

        Step = false;
        if (keysUp() & KEY_A)
        {
            printf("\x1b[0;0H");
            printf("\e[0;0H\e[2J");
            Step = true;
        }

        if (keysHeld() & KEY_Y)
            Step = true;

        if (keysUp() & KEY_Y)
        {
            printf("\x1b[0;0H");
            printf("\e[0;0H\e[2J");
        }

        if (Step)
        {
            StepCpu(&Cpu, CyclesToRun);
        }
        printf("\x1b[0;0H");
        DisassemblerPrintRange(&Cpu, Cpu.pc - (13 * 4), 29, Cpu.pc);

        gfxFlush(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL));
        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }

    // Exit services
    gfxExit();
    hidExit();
    return 0;
}




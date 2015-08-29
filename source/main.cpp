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

static void
std_out_puts(void *Memory, u32 Value)
{
    char *M = (char *)Memory;
    printf(M + (Value & 0x00FFFFFF));
}

int main(int argc, char **argv)
{
	if (argc > 1) chdir(argv[1]);
    gfxInitDefault();
    hidInit(NULL);
    PrintConsole BottomConsole;
    consoleInit(GFX_BOTTOM, &BottomConsole);

    MIPS_R3000 Cpu;
    GPU Gpu;
    Cpu.CP1 = &Gpu;
    MapRegister(&Cpu, (mmr) {GPU_GP0, &Gpu, GpuGp0});
    MapRegister(&Cpu, (mmr) {GPU_GP1, &Gpu, GpuGp1});
    MapRegister(&Cpu, (mmr) {0x1F802064, Cpu.Memory, std_out_puts});

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
    bool EnableDisassembler = false;
    bool AutoStep = true;
    while (aptMainLoop())
    {
        hidScanInput();
        u32 KeysDown = hidKeysDown();
        u32 KeysHeld = hidKeysHeld();
        u32 KeysUp = hidKeysUp();

        if (KeysDown & KEY_START)
            break;

        if (KeysDown & KEY_DDOWN)
        {
            ResetCpu(&Cpu);
        }

        if (KeysHeld & KEY_DLEFT)
        {
            CyclesToRun -= 1000;
            if (CyclesToRun < 1)
                CyclesToRun = 1;
        }

        if (KeysHeld & KEY_DRIGHT)
        {
            CyclesToRun += 1000;
        }

        if (KeysDown & KEY_DUP)
        {
            EnableDisassembler = !EnableDisassembler;
        }

        Step = false;
        if (KeysUp & KEY_A)
        {
            printf("\x1b[0;0H");
            printf("\e[0;0H\e[2J");
            Step = true;
        }

        if (KeysDown & KEY_Y)
            AutoStep = !AutoStep;

        if (Step || AutoStep)
        {
            StepCpu(&Cpu, CyclesToRun);
        }

        if (EnableDisassembler)
        {
            printf("\x1b[0;0H");
            DisassemblerPrintRange(&Cpu, Cpu.pc - (13 * 4), 29, Cpu.pc);
        }

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




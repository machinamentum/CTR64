
#include <3ds.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include "debugger.h"
#include "mips.h"
#include "gpu.h"
#include "disasm.h"

#define STAGE_IF 0
#define STAGE_DC 1
#define STAGE_EO 2
#define STAGE_MA 3
#define STAGE_WB 4

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

    FILE *f = fopen("psx_bios.bin", "rb");
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

    WriteMemWordRaw(&Cpu, 0xBFC003C4, 0x00000000);

    ResetCpu(&Cpu);

    opcode OpCodes[5];
    int Stages[5];

    for (int i = 4; i >= 0; --i)
    {
        Stages[i] = -(i + 1);
    }

    u32 MachineCodes[5];
#ifdef ENABLE_DEBUGGER
    if (DebuggerOpen())
    {
        printf("Could not start debugger client!\n");
    }
    else
    {
        printf("Started debugger\n");
    }

#endif

    bool Step = false;

    PrintConsole TopConsole;
    consoleInit(GFX_TOP, &TopConsole);
    consoleSelect(&BottomConsole);
    int CyclesToRun = 1;

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

        if (keysDown() & KEY_DDOWN)
        {
            for (int i = 4; i >= 0; --i)
            {
                Stages[i] = -(i + 1);
            }
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
            for (int t = 0; t < CyclesToRun; ++t)
            {
                for (int i  = 0; i < 5; ++i)
                {
                    if (Stages[i] < 0)
                    {
                        ++Stages[i];
                    }
                    
                    Stages[i] %= 5;

                    if (Stages[i] == STAGE_IF)
                    {
                        InstructionFetch(&Cpu, &MachineCodes[i]);
                        Stages[i]++;
                        continue;
                    }
                    if (Stages[i] == STAGE_DC)
                    {
                        DecodeOpcode(&Cpu, &OpCodes[i], MachineCodes[i], Cpu.pc - 4);
                        Stages[i]++;
                        continue;
                    }
                    if (Stages[i] == STAGE_EO)
                    {
                        ExecuteOpCode(&Cpu, &OpCodes[i]);
                        ExecuteWriteRegisters(&Cpu, &OpCodes[i]);
                        Stages[i]++;
                        continue;
                    }
                    if (Stages[i] == STAGE_MA)
                    {
                        MemoryAccess(&Cpu, &OpCodes[i]);
                        Stages[i]++;
                        continue;
                    }
                    if (Stages[i] == STAGE_WB)
                    {
                        WriteBack(&Cpu, &OpCodes[i]);
                        Stages[i]++;
                        continue;
                    }
                }
            }
        }
        printf("\x1b[0;0H");
        DisassemblerPrintRange(&Cpu, Cpu.pc - (13 * 4), 29, Cpu.pc);
        consoleSelect(&TopConsole);
        DumpState(&Cpu);
        for (int i = 0; i < 5; ++i)
        {
            printf("Stage %d: 0x%08X\n", i, Stages[i]);
        }
        printf("Cycles: %d\n", CyclesToRun);
        consoleSelect(&BottomConsole);

#ifdef ENABLE_DEBUGGER
        dbg_command Cmd;
        if (DebuggerGetCommand(&Cmd))
        {
            if (Cmd.Cmd == DEBUGGER_CMD_LOAD_KERNEL)
            {
                for (unsigned int i = 0; i < Cmd.PayloadSize; ++i)
                {
                    WriteMemByteRaw(&Cpu, RESET_VECTOR + i, ((char *)Cmd.Data)[i]);
                }
                for (int i = 4; i >= 0; --i)
                {
                    Stages[i] = -(i + 1);
                }
                ResetCpu(&Cpu);
                printf("\e[0;0H\e[2J");
            }
        }
#endif


        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }
#ifdef ENABLE_DEBUGGER
    DebuggerClose();
#endif

    // Exit services
    gfxExit();
    hidExit();
    return 0;
}





#include <3ds.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include "mips.h"

#define STAGE_IF 1
#define STAGE_DC 2
#define STAGE_EO 4
#define STAGE_MA 8
#define STAGE_WB 16

int main(int argc, char **argv)
{

    if (argc) chdir(argv[0]);

    gfxInitDefault();
    hidInit(NULL);
    consoleInit(GFX_BOTTOM, NULL);

    MIPS_R3000 Cpu;

    FILE *f = fopen("psx_bios.bin", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *BiosBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(BiosBuffer, fsize, 1, f);
    fclose(f);


    for (int i = 0; i < fsize; ++i)
    {
        WriteMemByte(&Cpu, RESET_VECTOR + i, BiosBuffer[i]);
    }

    opcode OpCodes[5];
    int Stages[5];

    for (int i = 4; i >= 0; --i)
    {
        Stages[i] = -i;
    }

    u32 MachineCode = 0;

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

        for (int i  = 0; i < 5; ++i)
        {
            if (Stages[i] < 1)
            {
                ++Stages[i];
            }

            if (Stages[i] == STAGE_IF)
            {
                InstructionFetch(&Cpu, &MachineCode);
                Stages[i] = STAGE_DC;
                continue;
            }
            if (Stages[i] == STAGE_DC)
            {
                DecodeOpcode(&Cpu, &OpCodes[i], MachineCode, Cpu.pc - 4);
                Stages[i] = STAGE_EO;
                continue;
            }
            if (Stages[i] == STAGE_EO)
            {
                ExecuteOpCode(&Cpu, &OpCodes[i]);
                Stages[i] = STAGE_MA;
                continue;
            }
            if (Stages[i] == STAGE_MA)
            {
                MemoryAccess(&Cpu, &OpCodes[i]);
                Stages[i] = STAGE_WB;
                continue;
            }
            if (Stages[i] == STAGE_WB)
            {
                WriteBack(&Cpu, &OpCodes[i]);
                Stages[i] = STAGE_IF;
                continue;
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }
    
    // Exit services
    gfxExit();
    hidExit();
    return 0;
}




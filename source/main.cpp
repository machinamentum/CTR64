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
#include "gpu.h"
#include "disasm.h"
#include "psxexe.h"
#include "joypad.h"
#include "hsf.h"

void
ResetCpu(MIPS_R3000 *Cpu)
{
    Cpu->pc = RESET_VECTOR;
}

static void
std_out_putchar(void *Ref, u32 Value)
{
    putchar(Value);
}

static u32
empty_ret(void *Obj, u32 Val)
{
    return 0;
}

static void
empty_write(void *Obj, u32 Address)
{

}

static hsf HSF;
static hsf_file *FilePtrs[15];
static int TakenFiles[15] = {};
static int ReturnFile = 0;
static int ReturnRead = 0;
static int ReturnSeek = 0;
static int ReturnFirstFile = 0;

static void
CTRXFileOpen(void *Ref, u32 Ptr)
{
    char *FileName = (char *)MapVirtualAddress((MIPS_R3000 *)Ref, Ptr);
    int FreeFile = -1;
    for (int i = 2; i < 15; ++i)
    {
        if (TakenFiles[i] == 0)
        {
            FreeFile = i;
        }
    }

    if (FreeFile > 1)
    {
        printf("CTRX: Opening file %s\n", FileName);
        FilePtrs[FreeFile] = HsfFileOpen(&HSF, FileName);
        if (!FilePtrs[FreeFile]->DirectoryEntry)
        {
            printf("Error opening file: %s\n", FileName);
            ReturnFile = -1;
            return;
        }

        TakenFiles[FreeFile] = 1;
    }

    ReturnFile = FreeFile;
}

static u32
CTRXFileOpenReturn(void *Ref, u32 Address)
{
    return ReturnFile;
}

static void
CTRXFileRead(void *Ref, u32 Ptr)
{
    MIPS_R3000 *Cpu = (MIPS_R3000 *)Ref;
    struct FReadInfo
    {
        int Fd;
        int Dst;
        int Length;
    } *FReadInfoPtr;
    FReadInfoPtr = (FReadInfo *)MapVirtualAddress(Cpu, Ptr);
    ReturnRead = FReadInfoPtr->Length;
    HsfFileRead(MapVirtualAddress(Cpu, FReadInfoPtr->Dst), 1, FReadInfoPtr->Length, FilePtrs[FReadInfoPtr->Fd]);
}

static u32
CTRXFileReadReturn(void *Ref, u32 Address)
{
    return ReturnRead;
}

static void
CTRXFileSeek(void *Ref, u32 Ptr)
{
    MIPS_R3000 *Cpu = (MIPS_R3000 *)Ref;
    struct FSeekInfo
    {
        int Fd;
        int Offset;
        int SeekType;
    } *FSeekInfoPtr;
    FSeekInfoPtr = (FSeekInfo *)MapVirtualAddress(Cpu, Ptr);
    HsfFileSeek(FilePtrs[FSeekInfoPtr->Fd], FSeekInfoPtr->Offset, FSeekInfoPtr->SeekType);
    ReturnSeek = HsfFileTell(FilePtrs[FSeekInfoPtr->Fd]);
}

static u32
CTRXFileSeekReturn(void *Ref, u32 Address)
{
    return ReturnSeek;
}

static void
CTRXFirstFile(void *Ref, u32 Ptr)
{
    MIPS_R3000 *Cpu = (MIPS_R3000 *)Ref;

    struct DirEntry
    {
        char FileName[0x14];
        int Attribute;
        int Size;
        void *Next;
        int SelectorNumber;
        int Reserved;
    };

    struct FFInfo
    {
        char *FileName;
        u32 Entry;
    } *FFInfoPtr;

    FFInfoPtr = (FFInfo *)MapVirtualAddress(Cpu, Ptr);
    DirEntry *Entry = (DirEntry *)MapVirtualAddress(Cpu, FFInfoPtr->Entry);
    snprintf(Entry->FileName, 0x14, "%s", (char *)MapVirtualAddress(Cpu, (u32)FFInfoPtr->FileName));

    printf("CTRX filefile: %s\n", Entry->FileName);

    auto GetFileSize = [](const char *FileName)
    {
        hsf_file *f = HsfFileOpen(&HSF, FileName);
        if (!f)
        {
            printf("Could not open file: %s\n", FileName);
            return 0L;
        }
        HsfFileSeek(f, 0, SEEK_END);
        long fsize = HsfFileTell(f);
        HsfFileSeek(f, 0, SEEK_SET);
        HsfFileClose(f);
        return fsize;
    };
    Entry->Size = GetFileSize(Entry->FileName);
    ReturnFirstFile = (int)FFInfoPtr->Entry;
}

static u32
CTRXFirstFileReturn(void *Ref, u32 Address)
{
    return ReturnFirstFile;
}

static u32 InterruptMask;

static void
CTRXInterruptRegisterWrite(void *Ref, u32 Value)
{
    InterruptMask = Value;
}

static u32
CTRXInterruptRegisterRead(void *Ref, u32 Address)
{
    return InterruptMask;
}

int main(int argc, char **argv)
{
    InitPlatform(argc, argv);

    HsfOpen(&HSF, "puzzle.hsf");

    MIPS_R3000 Cpu;
    GPU Gpu;
    Cpu.CP1 = &Gpu;
    MapRegister(&Cpu, (mmr) {GPU_GP0, &Gpu, GpuGp0, empty_ret});
    MapRegister(&Cpu, (mmr) {GPU_GP1, &Gpu, GpuGp1, GpuStat});
    MapRegister(&Cpu, (mmr) {0x1F802064, &Cpu, std_out_putchar, empty_ret});
    MapRegister(&Cpu, (mmr) {0x1F802068, &Cpu, CTRXFileOpen, CTRXFileOpenReturn});
    MapRegister(&Cpu, (mmr) {0x1F802070, &Cpu, CTRXFileRead, CTRXFileReadReturn});
    MapRegister(&Cpu, (mmr) {0x1F802074, &Cpu, CTRXFileSeek, CTRXFileSeekReturn});
    MapRegister(&Cpu, (mmr) {0x1F802078, &Cpu, CTRXFirstFile, CTRXFirstFileReturn});
    MapRegister(&Cpu, (mmr) {0x1F8010A0, &Cpu, DMA2Trigger, empty_ret});
    MapRegister(&Cpu, (mmr) {JOY_TX_DATA, nullptr, JoyTxWrite, JoyRxRead});
    MapRegister(&Cpu, (mmr) {0x1F801070, nullptr, empty_write, CTRXInterruptRegisterRead});
    MapRegister(&Cpu, (mmr) {0x1F801070, nullptr, CTRXInterruptRegisterWrite, empty_ret});

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

    HsfClose(&HSF);
    ExitPlatform();

    return 0;
}




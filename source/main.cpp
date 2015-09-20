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

struct FileAccessInfo
{
    FILE* Ptr;
    char *Name;
    int SeekPos;
};

static FileAccessInfo FilePtrs[15] = {};
static int TakenFiles[15] = {};
static int ReturnFile = 0;
static int ReturnRead = 0;
static int ReturnSeek = 0;
static int ReturnFirstFile = 0;
static const char *CDROMDir = "cdrom/";

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
        int Size = strlen(CDROMDir) + strlen(FileName) + 1;
        char *NameBuf = (char *)linearAlloc(Size + 1);
        snprintf(NameBuf, Size, "%s%s", CDROMDir, FileName);
        for (int i = 0; i < Size; ++i)
        {
            if (NameBuf[i] == '\\')
            {
                NameBuf[i] = '/';
            }
        }
        if (NameBuf[Size - 3] == ';') //version identifier
            NameBuf[Size - 3] = 0;
        printf("CTRX: Opening file %s\n", NameBuf);
        FilePtrs[FreeFile].Ptr = fopen(NameBuf, "rb");
        if (!FilePtrs[FreeFile].Ptr)
        {
            printf("Error opening file: %s\n", NameBuf);
            exit(-1);
        }
        FilePtrs[FreeFile].Name = NameBuf;
        FilePtrs[FreeFile].SeekPos = 0;
        fclose(FilePtrs[FreeFile].Ptr);
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
    FilePtrs[FReadInfoPtr->Fd].Ptr = fopen(FilePtrs[FReadInfoPtr->Fd].Name, "rb");
    fseek(FilePtrs[FReadInfoPtr->Fd].Ptr, FilePtrs[FReadInfoPtr->Fd].SeekPos, SEEK_SET);
    ReturnRead = fread(MapVirtualAddress(Cpu, FReadInfoPtr->Dst), 1, FReadInfoPtr->Length, FilePtrs[FReadInfoPtr->Fd].Ptr);
    FilePtrs[FReadInfoPtr->Fd].SeekPos += ReturnRead;
    fclose(FilePtrs[FReadInfoPtr->Fd].Ptr);
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
    FilePtrs[FSeekInfoPtr->Fd].Ptr = fopen(FilePtrs[FSeekInfoPtr->Fd].Name, "rb");
    fseek(FilePtrs[FSeekInfoPtr->Fd].Ptr, FilePtrs[FSeekInfoPtr->Fd].SeekPos, SEEK_SET);
    ReturnSeek = fseek(FilePtrs[FSeekInfoPtr->Fd].Ptr, FSeekInfoPtr->Offset, FSeekInfoPtr->SeekType);
    FilePtrs[FSeekInfoPtr->Fd].SeekPos = ReturnSeek;
    fclose(FilePtrs[FSeekInfoPtr->Fd].Ptr);
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
    int Size = strlen(Entry->FileName);
    for (int i = 0; i < Size; ++i)
    {
        if (Entry->FileName[i] == '\\')
        {
            Entry->FileName[i] = '/';
        }
        if (Entry->FileName[i] == ':')
        {
            Entry->FileName[i] = '/';
        }
    }
    if (Entry->FileName[Size - 2] == ';') //version identifier
        Entry->FileName[Size - 2] = 0;

    auto GetFileSize = [](const char *FileName)
    {
        FILE *f = fopen(FileName, "rb");
        if (!f)
        {
            printf("Could not open file: %s\n", FileName);
            return 0L;
        }
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        fclose(f);
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

int main(int argc, char **argv)
{
    InitPlatform(argc, argv);

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
    int CyclesToRun = 5000;
    bool EnableDisassembler = false;
    bool AutoStep = true;
    u32 IRQ0Steps = 0;
    while (MainLoopPlatform())
    {
#ifdef _3DS
        hidScanInput();
        u32 KeysDown = hidKeysDown();
        u32 KeysHeld = hidKeysHeld();
        u32 KeysUp = hidKeysUp();

        if (KeysDown & KEY_START)
            break;
#endif
//
//        if (KeysDown & KEY_DDOWN)
//        {
//            ResetCpu(&Cpu);
//        }
//
//        if (KeysHeld & KEY_DLEFT)
//        {
//            CyclesToRun -= 1000;
//            if (CyclesToRun < 1)
//                CyclesToRun = 1;
//        }
//
//        if (KeysHeld & KEY_DRIGHT)
//        {
//            CyclesToRun += 1000;
//        }
//
//        if (KeysDown & KEY_DUP)
//        {
//            EnableDisassembler = !EnableDisassembler;
//        }
//
//        Step = false;
//        if (KeysUp & KEY_A)
//        {
//            printf("\x1b[0;0H");
//            printf("\e[0;0H\e[2J");
//            Step = true;
//        }
//
//        if (KeysDown & KEY_Y)
//            AutoStep = !AutoStep;

        if (Step || AutoStep)
        {
            StepCpu(&Cpu, CyclesToRun);
            IRQ0Steps += CyclesToRun;
            if (IRQ0Steps > 550000)
            {
                C0GenerateException(&Cpu, 0, Cpu.pc - 4);
                Cpu.CP0.cause |= (1 << 8);
                IRQ0Steps = 0;
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




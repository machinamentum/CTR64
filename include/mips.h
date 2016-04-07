/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef MIPS_H
#define MIPS_H

#include "platform.h"

#define PRIMARY_OP_MASK \
(0b111111 << 26)

#define SECONDARY_OP_MASK \
(0b111111 << 0)

#define COMMENT20_MASK \
(0xFFFFF << 6)

#define IMM5_MASK \
(0b11111 << 6)

#define IMM16_MASK \
(0xFFFF << 0)

#define IMM25_MASK \
(0x1FFFFFF << 0)

#define IMM26_MASK \
(0x3FFFFFF << 0)

#define REG_RT_MASK \
(0b11111 << 16)

#define REG_RS_MASK \
(0b11111 << 21)

#define REG_RD_MASK \
(0b11111 << 11)

#define CODE10_MASK \
(0b1111111111 << 6)

#define KUSEG (0x00000000)
#define KSEG0 (0x80000000)
#define KSEG1 (0xA0000000)
#define KSEG2 (0xFFFE0000)
#define KSEGOFF (0x1F000000)

#define RESET_VECTOR (0xBFC00000)
#define UTLBM_VECTOR (0x80000000)
#define C0BRK_VECTOR (0x80000040)
#define GNRAL_VECTOR (0x80000080)

#define C0_PRID_VALUE  (0x00000002)

#define C0_BVA    9
#define C0_STATUS 13
#define C0_CAUSE  14
#define C0_EPC    15
#define C0_PRID   16

#define C0_STATUS_IEc    (1 << 0)
#define C0_STATUS_KUc    (1 << 1)
#define C0_STATUS_IEp    (1 << 2)
#define C0_STATUS_KUp    (1 << 3)
#define C0_STATUS_IEo    (1 << 4)
#define C0_STATUS_KUo    (1 << 5)
#define C0_STATUS_Im  (0xFF << 8)
#define C0_STATUS_Isc   (1 << 16)
#define C0_STATUS_Swc   (1 << 17)
#define C0_STATUS_PZ    (1 << 18)
#define C0_STATUS_CM    (1 << 19)
#define C0_STATUS_PE    (1 << 20)
#define C0_STATUS_TS    (1 << 21)
#define C0_STATUS_BEV   (1 << 22)
#define C0_STATUS_RE    (1 << 25)
#define C0_STATUS_CU0   (1 << 28)
#define C0_STATUS_CU1   (1 << 29)
#define C0_STATUS_CU2   (1 << 30)
#define C0_STATUS_CU3   (1 << 31)


#define C0_CAUSE_INT     0
#define C0_CAUSE_ADDRL   4
#define C0_CAUSE_ADDRS   5
#define C0_CAUSE_IBUS    6
#define C0_CAUSE_DBUS    7
#define C0_CAUSE_SYSCALL 8
#define C0_CAUSE_BKPT    9
#define C0_CAUSE_RI     10
#define C0_CAUSE_OVF    11

#define C0_CAUSE_MASK  (0b11111 << 2)

#define MIPS_MODE_KERNEL 1
#define MIPS_MODE_USER   0

#define REG_INDEX_RA 31
#define REG_INDEX_PC 32


#define MEM_ACCESS_NONE   0
#define MEM_ACCESS_WRITE  1
#define MEM_ACCESS_READ   2

#define MEM_ACCESS_BYTE   4
#define MEM_ACCESS_HALF   8
#define MEM_ACCESS_WORD   16
#define MEM_ACCESS_DWORD  32

#define MEM_ACCESS_SIGNED 64
#define MEM_ACCESS_HIGH   128
#define MEM_ACCESS_LOW    256

struct MIPS_R3000;

struct opcode
{
    u64 CurrentAddress;

    u64 MemAccessAddress;
    u64 MemAccessValue;
    u32 MemAccessMode;
};

struct Coprocessor
{
    union
    {
        u64 registers[64];
        struct
        {
            u64 r0;
            u64 r1;
            u64 r2;
            u64 bpc;
            u64 r4;
            u64 bda;
            u64 jumpdest;
            u64 dcic;
            u64 bva;
            u64 bdam;
            u64 r10;
            u64 bpcm;
            u64 sr;
            u64 cause;
            u64 epc;
            u64 prid = C0_PRID_VALUE;
        };
    };
    Coprocessor() {}
    void (*ExecuteOperation)(Coprocessor *Cp, u32 FunctionCode) = nullptr;
};

struct mmr
{
    u64 Address;
    void *Object;
    void (*RegisterWriteFunc)(void *, u64 Value);
    u32  (*RegisterReadFunc)(void *, u64 Address);
};

struct mmm
{
    void *Ptr;
    u64 VirtualAddress;
    u32 Size;
};

struct DMA
{
    u32 MADR;
    u32 BCR;
    u32 CHCR;
    u32 Empty;
};

struct MIPS_R3000
{
    union
    {
        u64 GPR[35];
        struct
        {
            u64 zero;
            u64 at;

            u64 v0;
            u64 v1;

            u64 a0;
            u64 a1;
            u64 a2;
            u64 a3;

            u64 t0;
            u64 t1;
            u64 t2;
            u64 t3;
            u64 t4;
            u64 t5;
            u64 t6;
            u64 t7;

            u64 s0;
            u64 s1;
            u64 s2;
            u64 s3;
            u64 s4;
            u64 s5;
            u64 s6;
            u64 s7;
            u64 t8;
            u64 t9;

            u64 k0;
            u64 k1;

            u64 gp;
            u64 sp;
            u64 fp;

            u64 ra;
            u64 pc = RESET_VECTOR;

            u64 hi, lo;

            u32 fcr0;
            u32 frc31;

            u32 llbit;
        };
    };

    double FGR[32];

    u32 SkipExecute = 0; // Skips execution of a single instruction
    opcode OpCodes[2];
    u32 BaseState = 0;
    void *NextJump;
    u32 NextData;
    u32 NumMMR;
    mmr MemMappedRegisters[0x10];
    u32 NumMMM;
    mmm MemMappedMemRegions[0x10];

    MIPS_R3000();

    u32 DPCR;
    u32 DICR;
    Coprocessor CP0;
    Coprocessor *CP1 = NULL;
    Coprocessor *CP2 = NULL;
    Coprocessor *CP3 = NULL;
    DMA DMAChannels[8];
};

void MapRegister(MIPS_R3000 *Cpu, mmr MMR);
void StepCpu(MIPS_R3000 *Cpu, u32 Steps);
void C0GenerateException(MIPS_R3000 *, u8, u64);
void MapMemoryRegion(MIPS_R3000 *, mmm);

inline void *
MapVirtualAddress(MIPS_R3000 *Cpu, u64 Address)
{
    Address = Address & 0x1FFFFFFF;
    for (u32 i = 0; i < Cpu->NumMMM; ++i)
    {
        mmm *MMM = &Cpu->MemMappedMemRegions[i];
        u64 Addr = MMM->VirtualAddress & 0x1FFFFFFF;
        u32 Size = MMM->Size;
        if ( (Address >= Addr) && (Address < (Addr + Size)) ) {
            return ((u8 *)MMM->Ptr) + (Address - Addr);
        }
    }
    return nullptr;
}

inline u64
ReadMemDWordRaw(MIPS_R3000 *Cpu, u64 Address)
{
    void *Addr = MapVirtualAddress(Cpu, Address);
    if (Addr)
    {
        u64 Value = *((u64 *)((u8 *)Addr));
        return ((Cpu->CP0.sr & C0_STATUS_RE) ? Value: __builtin_bswap64(Value));
    }
    return 0;
}

inline u32
ReadMemWordRaw(MIPS_R3000 *Cpu, u64 Address)
{
    void *Addr = MapVirtualAddress(Cpu, Address);
    if (Addr)
    {
        u32 Value = *((u32 *)((u8 *)Addr));
        return ((Cpu->CP0.sr & C0_STATUS_RE) ? Value: __builtin_bswap32(Value));
    }
    return 0;
}

inline u8
ReadMemByteRaw(MIPS_R3000 *Cpu, u64 Address)
{
    void *Addr = MapVirtualAddress(Cpu, Address);
    if (Addr)
    {
        return *((u8 *)Addr);
    }

    return 0;
}

inline u16
ReadMemHalfWordRaw(MIPS_R3000 *Cpu, u64 Address)
{
    void *Addr = MapVirtualAddress(Cpu, Address);
    if (Addr)
    {
        u16 Value = *((u16 *)((u8 *)Addr));
        return ((Cpu->CP0.sr & C0_STATUS_RE) ? Value: __builtin_bswap16(Value));
    }
    return 0;
}

inline void
WriteMemByteRaw(MIPS_R3000 *Cpu, u64 Address, u8 Value)
{
    *((u8 *)MapVirtualAddress(Cpu, Address)) = Value;
}

inline void
WriteMemWordRaw(MIPS_R3000 *Cpu, u32 Address, u32 Value)
{
    *((u32 *)((u8 *)MapVirtualAddress(Cpu, Address))) = ((Cpu->CP0.sr & C0_STATUS_RE) ? Value: __builtin_bswap32(Value));
}

inline void
WriteMemHalfWordRaw(MIPS_R3000 *Cpu, u32 Address, u16 Value)
{
    *((u16 *)((u8 *)MapVirtualAddress(Cpu, Address))) = ((Cpu->CP0.sr & C0_STATUS_RE) ? Value: __builtin_bswap16(Value));
}

inline u64
SignExtend32To64(s64 i)
{
    return (u64)(s64)(s32)i;
}

inline u32
SignExtend16(s32 i)
{
    return (u32)(s32)(s16)i;
}

inline u64
SignExtend16To64(s64 i)
{
    return (u64)(s64)(s16)i;
}

inline u32
SignExtend8(s8 i)
{
    return (u32)(s32)i;
}

inline u64
SignExtend8To64(s64 i)
{
    return (u64)(s64)(s8)i;
}


#endif

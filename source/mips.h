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

#include <3ds.h>
#include <3ds/types.h>

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
#define REG_INDEX_HL 33


#define MEM_ACCESS_NONE   0
#define MEM_ACCESS_BRANCH 1
#define MEM_ACCESS_WRITE  2
#define MEM_ACCESS_READ   3

#define MEM_ACCESS_BYTE   2
#define MEM_ACCESS_HALF   1
#define MEM_ACCESS_WORD   0

#define WRITE_BACK_CPU    0
#define WRITE_BACK_C0     1
#define WRITE_BACK_C1     2
#define WRITE_BACK_C2     3
#define WRITE_BACK_C3     4

struct opcode
{
    u32 LeftValue = 0;
    u32 RightValue;
    u32 Immediate;
    u32 Result;
    u32 Select0;
    u32 Select1;
    u32 FunctionSelect;
    u32 MemAccessType;
    u32 MemAccessMode;
    u32 WriteBackMode;
    u32 DestinationRegister;
    u32 RADestinationRegister; // Used for return address writing
    u32 CurrentAddress;
};

struct Coprocessor
{
    union
    {
        u32 registers[64];
        struct
        {
            u32 r0;
            u32 r1;
            u32 r2;
            u32 bpc;
            u32 r4;
            u32 bda;
            u32 jumpdest;
            u32 dcic;
            u32 bva;
            u32 bdam;
            u32 r10;
            u32 bpcm;
            u32 sr;
            u32 cause;
            u32 epc;
            u32 prid = C0_PRID_VALUE;
        };
    };

    void (*ExecuteOperation)(Coprocessor *Cp, u32 FunctionCode) = NULL;
};

struct mmr
{
    u32 Address;
    void *Object;
    void (*RegisterWriteFunc)(void *, u32 Value);
    u32  (*RegisterReadFunc)(void *, u32 Address);
};

struct MIPS_R3000
{
    union
    {
        u32 registers[35];
        struct
        {
            u32 zero;
            u32 at;

            u32 v0;
            u32 v1;

            u32 a0;
            u32 a1;
            u32 a2;
            u32 a3;

            u32 t0;
            u32 t1;
            u32 t2;
            u32 t3;
            u32 t4;
            u32 t5;
            u32 t6;
            u32 t7;

            u32 s0;
            u32 s1;
            u32 s2;
            u32 s3;
            u32 s4;
            u32 s5;
            u32 s6;
            u32 s7;
            u32 t8;
            u32 t9;

            u32 k0;
            u32 k1;

            u32 gp;
            u32 sp;
            u32 fp;

            u32 ra;
            u32 pc = RESET_VECTOR;

            u32 hi, lo;
        };
    };

    opcode OpCodes[4];
    u32 MachineCode = 0;
    u32 BaseState = 0;

    MIPS_R3000();

    void *Memory = linearAlloc(0x1000000);
    Coprocessor CP0;
    Coprocessor *CP1 = NULL;
    Coprocessor *CP2 = NULL;
    Coprocessor *CP3 = NULL;
    mmr MemMappedRegisters[64];
    u32 NumMMR = 0;
};

void MapRegister(MIPS_R3000 *Cpu, mmr MMR);
void StepCpu(MIPS_R3000 *Cpu, u32 Steps);

inline u32
ReadMemWordRaw(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u32 *)((u8 *)Cpu->Memory + Base));
}

inline u8
ReadMemByteRaw(MIPS_R3000 *Cpu, u32 Address, u8 value)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u8 *)Cpu->Memory + Base);
}

inline u16
ReadMemHalfWordRaw(MIPS_R3000 *Cpu, u32 Address, u16 value)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u16 *)((u8 *)Cpu->Memory + Base));
}

inline void
WriteMemByteRaw(MIPS_R3000 *Cpu, u32 Address, u8 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u8 *)Cpu->Memory + Base) = value;
}

inline void
WriteMemWordRaw(MIPS_R3000 *Cpu, u32 Address, u32 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u32 *)((u8 *)Cpu->Memory + Base)) = value;
}

inline void
WriteMemHalfWordRaw(MIPS_R3000 *Cpu, u32 Address, u16 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u16 *)((u8 *)Cpu->Memory + Base)) = value;
}

inline u32
SignExtend16(s16 i)
{
    return (u32)(s32)i;
}

inline u32
SignExtend8(s8 i)
{
    return (u32)(s32)i;
}


#endif

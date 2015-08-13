#ifndef MIPS_H
#define MIPS_H

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

#define C0_PRID  (0x00000002)

#define C0_BVA    8
#define C0_STATUS 12
#define C0_CAUSE  13
#define C0_EPC    14

#define C0_CAUSE_INT     0
#define C0_CAUSE_ADDRL   4
#define C0_CAUSE_ADDRS   5
#define C0_CAUSE_IBUS    6
#define C0_CAUSE_DBUS    7
#define C0_CAUSE_SYSCALL 8
#define C0_CAUSE_BKPT    9
#define C0_CAUSE_RI     10
#define C0_CAUSE_OVF    11

#define C0_CAUSE_MASK  (0b111111 << 10)

#define MIPS_MODE_KERNEL 1
#define MIPS_MODE_USER   0

#define REG_INDEX_RA 31
#define REG_INDEX_PC 32
#define REG_INDEX_HL 33


#define MEM_ACCESS_NONE   0
#define MEM_ACCESS_BRANCH 1
#define MEM_ACCESS_WRITE  2
#define MEM_ACCESS_READ   4

#define MEM_ACCESS_BYTE   1
#define MEM_ACCESS_HALF   2
#define MEM_ACCESS_WORD   4


struct opcode
{
    u32 CurrentAddress = 0;
    u8 Select0;
    u8 Select1;
    u8 MemAccessType = MEM_ACCESS_NONE;
    u8 MemAccessMode = MEM_ACCESS_WORD;

    u32 LeftValue;
    u32 RightValue;
    u32 Immediate;
    u32 Result;

    u8 DestinationRegister = 0;
    u8 RADestinationRegister = 0; // Used for return address writing
};

struct C0Processor
{
    union
    {
        u32 registers[15];
        struct
        {
            u32 pad[7];
            u32 bva;
            u32 status;
            u32 cause;
            u32 epc;
            const u32 prid = C0_PRID;
        };
    };
};

struct MIPS_R3000
{
    union
    {
        u32 registers[34];
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
            u16 hi, lo;
        };
    };

    void *Memory = linearAlloc(0x1000000);
    int KMode;
    C0Processor CP0;
    
};

void InstructionFetch(MIPS_R3000 *Cpu, u32 *Code);
void DecodeOpcode(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data, u32 IAddress);
void ExecuteOpCode(MIPS_R3000 *Cpu, opcode *OpCode);
void MemoryAccess(MIPS_R3000 *Cpu, opcode *OpCode);
void WriteBack(MIPS_R3000 *Cpu, opcode *OpCode);

inline u32
MemReadWord(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u32 *)((u8 *)Cpu->Memory + Base));
}

inline void
WriteMemByte(MIPS_R3000 *Cpu, u32 Address, u8 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u8 *)Cpu->Memory + Base) = value;
}

inline void
WriteMemWord(MIPS_R3000 *Cpu, u32 Address, u32 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u32 *)((u8 *)Cpu->Memory + Base)) = value;
}

inline void
WriteMemHalfWord(MIPS_R3000 *Cpu, u32 Address, u16 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u16 *)((u8 *)Cpu->Memory + Base)) = value;
}


#endif

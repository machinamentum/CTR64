/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <cstdio>
#include "mips.h"



inline u32
ReadMemWord(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        mmr *MMR = &Cpu->MemMappedRegisters[i];
        if (Base == MMR->Address)
        {

            return MMR->RegisterReadFunc(MMR->Object, Address);
        }
    }
    return *((u32 *)((u8 *)MapVirtualAddress(Cpu, Base)));
}

inline u8
ReadMemByte(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        mmr *MMR = &Cpu->MemMappedRegisters[i];
        if (Base == MMR->Address)
        {

            return MMR->RegisterReadFunc(MMR->Object, Address);
        }
    }
    return *((u8 *)MapVirtualAddress(Cpu, Base));
}

inline u16
ReadMemHalfWord(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        mmr *MMR = &Cpu->MemMappedRegisters[i];
        if (Base == MMR->Address)
        {

            return MMR->RegisterReadFunc(MMR->Object, Address);
        }
    }
    return *((u16 *)((u8 *)MapVirtualAddress(Cpu, Base)));
}

inline void
WriteMemByte(MIPS_R3000 *Cpu, u32 Address, u8 value)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        mmr *MMR = &Cpu->MemMappedRegisters[i];
        if (Base == MMR->Address)
        {
            
            MMR->RegisterWriteFunc(MMR->Object, value);
            return;
        }
    }
    *((u8 *)MapVirtualAddress(Cpu, Base)) = value;
}

inline void
WriteMemWord(MIPS_R3000 *Cpu, u32 Address, u32 value)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        mmr *MMR = &Cpu->MemMappedRegisters[i];
        if (Base == MMR->Address)
        {
            MMR->RegisterWriteFunc(MMR->Object, value);
            return;
        }
    }
    *((u32 *)((u8 *)MapVirtualAddress(Cpu, Base))) = value;
}

inline void
WriteMemHalfWord(MIPS_R3000 *Cpu, u32 Address, u16 value)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        mmr *MMR = &Cpu->MemMappedRegisters[i];
        if (Base == MMR->Address)
        {
            MMR->RegisterWriteFunc(MMR->Object, value);
            return;
        }
    }
    *((u16 *)((u8 *)MapVirtualAddress(Cpu, Base))) = value;
}

static void
C0ExecuteOperation(Coprocessor *Cp, u32 FunctionCode);

MIPS_R3000::
MIPS_R3000()
{
    CP0.ExecuteOperation = C0ExecuteOperation;
    RAM = linearAlloc(2048 * 1000);
    BIOS = linearAlloc(512 * 1000);
    Dummy = linearAlloc(512);
    NumMMR = 0;
}

//Exceptions
inline void
C0ExceptionPushSRBits(Coprocessor *CP0)
{
    u32 SR = CP0->sr;
    u32 IEp = (SR >> 2) & 1;
    u32 KUp = (SR >> 3) & 1;
    u32 IEc = (SR) & 1;
    u32 KUc = (SR >> 1) & 1;
    SR ^= (-IEp ^ SR) & (1 << 4);
    SR ^= (-KUp ^ SR) & (1 << 5);
    SR ^= (-IEc ^ SR) & (1 << 2);
    SR ^= (-KUc ^ SR) & (1 << 3);
    CP0->sr = SR;
}

inline void
C0ExceptionPopSRBits(Coprocessor *CP0)
{
    u32 SR = CP0->sr;
    u32 IEp = (SR >> 2) & 1;
    u32 KUp = (SR >> 3) & 1;
    u32 IEo = (SR >> 4) & 1;
    u32 KUo = (SR >> 5) & 1;
    SR ^= (-IEp ^ SR) & (1 << 0);
    SR ^= (-KUp ^ SR) & (1 << 1);
    SR ^= (-IEo ^ SR) & (1 << 2);
    SR ^= (-KUo ^ SR) & (1 << 3);
    CP0->sr = SR;
}

void
C0GenerateException(MIPS_R3000 *Cpu, u8 Cause, u32 EPC)
{
    if (Cpu->CP0.sr & C0_STATUS_IEc)
    {
        Cpu->CP0.cause = (Cause << 2) & C0_CAUSE_MASK;
        Cpu->CP0.epc = EPC;
        C0ExceptionPushSRBits(&Cpu->CP0);
        Cpu->CP0.sr &= ~C0_STATUS_KUc;
        Cpu->pc = GNRAL_VECTOR;
    }
}

static void
C0ReturnFromException(Coprocessor *Cp)
{
    C0ExceptionPopSRBits(Cp);
}

static void
C0ExecuteOperation(Coprocessor *Cp, u32 FunctionCode)
{
    if (FunctionCode == 0x10)
    {
        C0ReturnFromException(Cp);
    }
}

static void
ReservedInstructionException(MIPS_R3000 *Cpu, opcode *Op)
{
    C0GenerateException(Cpu, C0_CAUSE_RI, Op->CurrentAddress);
}

static void
SysCall(MIPS_R3000 *Cpu, opcode *OpCode)
{
//    u32 Immediate = (Data & COMMENT20_MASK) >> 6;
    C0GenerateException(Cpu, C0_CAUSE_SYSCALL, OpCode->CurrentAddress);
}

static void
Break(MIPS_R3000 *Cpu, opcode *OpCode)
{
//    OpCode->Immediate = (Data & COMMENT20_MASK) >> 6;
    C0GenerateException(Cpu, C0_CAUSE_BKPT, OpCode->CurrentAddress);
}

typedef void (*jt_func)(MIPS_R3000 *, opcode *);

static jt_func PrimaryJumpTable[0x40];
static jt_func SecondaryJumpTable[0x40];

//Arithmetic
static void
AddU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] + Cpu->registers[rt];
    }
}

static void
AddIU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->registers[rt] = Cpu->registers[rs] + Immediate;
    }
}

static void
SubU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] - Cpu->registers[rt];
    }
}

static void
Add(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] + Cpu->registers[rt];
    }
    // TODO overflow trap
}

static void
AddI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->registers[rt] = Cpu->registers[rs] + Immediate;
    }
    // TODO overflow trap
}

static void
Sub(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] - Cpu->registers[rt];
    }
    // TODO overflow trap
}

//HI:LO operations
static void
MFHI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;
    Cpu->registers[rd] = Cpu->hi;
}

static void
MFLO(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;
    Cpu->registers[rd] = Cpu->lo;
}

static void
MTHI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    Cpu->hi = rs;
}

static void
MTLO(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    Cpu->lo = rs;
}

static void
Mult(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    s64 Result = (s64)Cpu->registers[rs] * (s64)Cpu->registers[rt];
    Cpu->hi = (Result >> 32) & 0xFFFFFFFF;
    Cpu->lo = Result & 0xFFFFFFFF;
}

static void
MultU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    u64 Result = (u64)Cpu->registers[rs] * (u64)Cpu->registers[rt];
    Cpu->hi = Result >> 32;
    Cpu->lo = Result & 0xFFFFFFFF;
}

static void
Div(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    s32 Left = Cpu->registers[rs];
    s32 Right = Cpu->registers[rt];
    if (!Right)
    {
        Cpu->hi = Left;
        if (Left >= 0)
        {
            Cpu->lo = -1;
        }
        else
        {
            Cpu->lo = 1;
        }
        return;
    }
    if (Right == -1 && (u32)Left == 0x80000000)
    {
        Cpu->hi = 0;
        Cpu->lo = 0x80000000;
        return;
    }
    Cpu->lo = Left / Right;
    Cpu->hi = Left % Right;
}

static void
DivU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    u32 Left = Cpu->registers[rs];
    u32 Right = Cpu->registers[rt];
    if (!Right)
    {
        Cpu->hi = Left;
        Cpu->lo = 0xFFFFFFFF;
        return;
    }
    Cpu->lo = Left / Right;
    Cpu->hi = Left % Right;
}

//Store
static void
SW(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u32 Immediate = SignExtend16((Data & IMM16_MASK));
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->registers[rt];
    OpCode->MemAccessMode = MEM_ACCESS_WORD | MEM_ACCESS_WRITE;
}

static void
SH(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u32 Immediate = SignExtend16((Data & IMM16_MASK));
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->registers[rt];
    OpCode->MemAccessMode = MEM_ACCESS_HALF | MEM_ACCESS_WRITE;
}

static void
SB(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u32 Immediate = SignExtend16((Data & IMM16_MASK));
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->registers[rt];
    OpCode->MemAccessMode = MEM_ACCESS_BYTE | MEM_ACCESS_WRITE;
}

//Load
static void
LUI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 Immediate = (Data & IMM16_MASK) >> 0;
        Cpu->registers[rt] = Immediate << 16;
    }
}

static void
LW(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_WORD;
    }
}

static void
LBU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_BYTE;
    }
}

static void
LHU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_HALF;
    }
}

static void
LB(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_BYTE | MEM_ACCESS_SIGNED;
    }
}

static void
LH(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->registers[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_HALF | MEM_ACCESS_SIGNED;
    }
}


// Jump/Call
static void
J(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u32 Immediate = (Data & IMM26_MASK) >> 0;
    Cpu->pc = (Cpu->pc & 0xF0000000) + (Immediate * 4);
}

static void
JAL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u32 Immediate = (Data & IMM26_MASK) >> 0;
    Cpu->ra = OpCode->CurrentAddress + 8;
    Cpu->pc = (Cpu->pc & 0xF0000000) + (Immediate * 4);
}

static void
JR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    Cpu->pc = Cpu->registers[rs];
}

static void
JALR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rd = (Data & REG_RD_MASK) >> 11;
    Cpu->pc = Cpu->registers[rs];
    if (rd) Cpu->registers[rd] = OpCode->CurrentAddress + 8;
}

static void
BranchZero(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);

    //bltz, bgez, bltzal, bgezal
    u8 type = rt;
    u32 Address = OpCode->CurrentAddress + 4 + Immediate * 4;
    s32 Check = Cpu->registers[rs];

    if (type & 0b00001)
    {
        //bgez
        if (Check >= 0)
        {
            if (type & 0b10000)
            {
                Cpu->ra = OpCode->CurrentAddress + 8;
            }
            Cpu->pc = Address;
        }
    }
    else
    {
        //bltz
        if (Check < 0)
        {
            if (type & 0b10000)
            {
                Cpu->ra = OpCode->CurrentAddress + 8;
            }
            Cpu->pc = Address;
        }
    }
}

static void
BEQ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    
    if (Cpu->registers[rs] == Cpu->registers[rt])
    {
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BNE(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (Cpu->registers[rs] != Cpu->registers[rt])
    {
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BLEZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;

    if (Cpu->registers[rs] <= 0)
    {
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BGTZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;

    if (Cpu->registers[rs] > 0)
    {
        u32 Immediate = SignExtend16((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

//Logical
static void
AndI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = Data & IMM16_MASK;
        Cpu->registers[rt] = Cpu->registers[rs] & Immediate;
    }
}

static void
OrI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = Data & IMM16_MASK;
        Cpu->registers[rt] = Cpu->registers[rs] | Immediate;
    }
}

static void
And(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] & Cpu->registers[rt];
    }
}

static void
Or(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] | Cpu->registers[rt];
    }
}

static void
XOr(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rs] ^ Cpu->registers[rt];
    }
}
static void
NOr(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = 0xFFFFFFFF ^ (Cpu->registers[rs] | Cpu->registers[rt]);
    }
}

static void
XOrI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = Data & IMM16_MASK;
        Cpu->registers[rt] = Cpu->registers[rs] ^ Immediate;
    }
}

//shifts
static void
SLLV(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rt] << (Cpu->registers[rs] & 0x1F);
    }
}

static void
SRLV(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rt] >> (Cpu->registers[rs] & 0x1F);
    }
}

static void
SRAV(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = ((s32)Cpu->registers[rt]) >> (Cpu->registers[rs] & 0x1F);
    }
}

static void
SLL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 Immediate = (Data & IMM5_MASK) >> 6;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rt] << Immediate;
    }
}

static void
SRL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 Immediate = (Data & IMM5_MASK) >> 6;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = Cpu->registers[rt] >> Immediate;
    }
}

static void
SRA(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 Immediate = (Data & IMM5_MASK) >> 6;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = ((s32)Cpu->registers[rt]) >> Immediate;
    }
}

// comparison
static void
SLT(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = ( ((s32)Cpu->registers[rs] < (s32)Cpu->registers[rt]) ? 1 : 0);
    }
}

static void
SLTU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u8 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->registers[rd] = ( (Cpu->registers[rs] < Cpu->registers[rt]) ? 1 : 0);
    }
}

static void
SLTI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->registers[rt] = ( ((s32)Cpu->registers[rs] < (s32)Immediate) ? 1 : 0);
    }
}

static void
SLTIU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u8 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->registers[rt] = ( (Cpu->registers[rs] < Immediate) ? 1 : 0);
    }
}

// coprocessor ops
static void
COP0(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    Coprocessor *CP0 = &Cpu->CP0;

    if (rs < 0b00100)
    {
        CP0->registers[rd + (rs & 0b00010 ? 32 : 0)] = Cpu->registers[rt];
    }
    else if (rs < 0b10000)
    {
        if (rs < 0b01000)
        {
            if (rt) Cpu->registers[rt] = CP0->registers[rd + (rs & 0b00010 ? 32 : 0)];
        }
        else
        {
            u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
            if (rt)
            {
                if (CP0->sr & C0_STATUS_CU0)
                {
                    Cpu->pc = OpCode->CurrentAddress + Immediate;
                }
            }
            else
            {
                if ((CP0->sr & C0_STATUS_CU0) == 0)
                {
                    Cpu->pc = OpCode->CurrentAddress + Immediate;
                }
            }
        }
    }
    else
    {
        CP0->ExecuteOperation(CP0, (Data & IMM25_MASK));
    }
}

static void
COP1(MIPS_R3000 *Cpu, opcode *OpCode)
{
    // PSX missing cop1
    // TODO exceptions
}

static void
COP2(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Data = OpCode->Data;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    Coprocessor *CP2 = Cpu->CP2;
    if (rs < 0b00100)
    {
        CP2->registers[rd + (rs & 0b00010 ? 32 : 0)] = Cpu->registers[rt];
    }
    else if (rs < 0b10000)
    {
        if (rs < 0b01000)
        {
            if (rt) Cpu->registers[rt] = CP2->registers[rd + (rs & 0b00010 ? 32 : 0)];
        }
        else
        {
            u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
            if (rt)
            {
                if (CP2->sr & C0_STATUS_CU2)
                {
                    Cpu->pc = OpCode->CurrentAddress + Immediate;
                }
            }
            else
            {
                if ((CP2->sr & C0_STATUS_CU2) == 0)
                {
                    Cpu->pc = OpCode->CurrentAddress + Immediate;
                }
            }
        }
    }
    else
    {
        CP2->ExecuteOperation(CP2, (Data & IMM25_MASK));
    }
}

static void
COP3(MIPS_R3000 *Cpu, opcode *OpCode)
{
    // PSX missing cop3
    // TODO exceptions
}


void
DecodeOpcode(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 Select0 = (Data & PRIMARY_OP_MASK) >> 26;

    if (Select0)
    {
        OpCode->ExecuteFunc = PrimaryJumpTable[Select0];
    }
    else
    {
        u32 Select1 = (Data & SECONDARY_OP_MASK);
        OpCode->ExecuteFunc = SecondaryJumpTable[Select1];
    }

    OpCode->CurrentAddress = Cpu->IAddress;
    OpCode->Data = Data;
    OpCode->MemAccessMode = MEM_ACCESS_NONE;



//    //lwc
//    else if ((Select0 & 0b111000) == 0b110000)
//    {
//        OpCode->WriteBackMode = Select0 & 0b111;
//        OpCode->LeftValue = Cpu->registers[rs];
//        OpCode->DestinationRegister = rt;
//        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
//        OpCode->MemAccessType = MEM_ACCESS_READ;
//    }
//    //swc
//    else if ((Select0 & 0b111000) == 0b111000)
//    {
//        OpCode->LeftValue = Cpu->registers[rs];
//        OpCode->RightValue = Cpu->CP0.registers[rt];
//        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
//        OpCode->MemAccessType = MEM_ACCESS_WRITE;
//    }
}

inline u32
InstructionFetch(MIPS_R3000 *Cpu)
{
    u32 Result = ReadMemWord(Cpu, Cpu->pc);
    Cpu->pc += 4;
    return Result;
}

inline void
ExecuteOpCode(MIPS_R3000 *Cpu, opcode *OpCode)
{
    jt_func Func = OpCode->ExecuteFunc;
    if (Func)
    {
        Func(Cpu, OpCode);
    }
}

void
MemoryAccess(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 Address = OpCode->MemAccessAddress;
    u32 Value = OpCode->MemAccessValue;
    u32 MemAccessMode = OpCode->MemAccessMode;

    if (MemAccessMode & MEM_ACCESS_WRITE)
    {
        if (MemAccessMode & MEM_ACCESS_BYTE)
        {
            WriteMemByte(Cpu, Address, Value);
        }

        else if (MemAccessMode & MEM_ACCESS_HALF)
        {
            WriteMemHalfWord(Cpu, Address, Value);
        }

        else if (MemAccessMode & MEM_ACCESS_WORD)
        {
            WriteMemWord(Cpu, Address, Value);
        }
    }
    else if (MemAccessMode & MEM_ACCESS_READ)
    {
        if (Value)
        {
            u32 Register = Value;
            u32 Signed = MemAccessMode & MEM_ACCESS_SIGNED;
            if (MemAccessMode & MEM_ACCESS_BYTE)
            {
                Value = ReadMemByte(Cpu, Address);
                if (Signed)
                {
                    Value = SignExtend8(Value);
                }
            }

            else if (MemAccessMode & MEM_ACCESS_HALF)
            {
                Value = ReadMemHalfWord(Cpu, Address);
                if (Signed)
                {
                    Value = SignExtend16(Value);
                }
            }

            else if (MemAccessMode & MEM_ACCESS_WORD)
            {
                Value = ReadMemWord(Cpu, Address);
            }
            Cpu->registers[Register] = Value;
        }
    }
}

void
MapRegister(MIPS_R3000 *Cpu, mmr MMR)
{
    Cpu->MemMappedRegisters[Cpu->NumMMR] = MMR;
    Cpu->MemMappedRegisters[Cpu->NumMMR].Address &= 0x00FFFFFF;
    ++Cpu->NumMMR;
}

#define STAGE_IF 0
#define STAGE_DC 1
#define STAGE_EO 2
#define STAGE_MA 3
#define STAGE_WB 4

void
StepCpu(MIPS_R3000 *Cpu, u32 Steps)
{
    u32 BS = Cpu->BaseState;
    for (u32 t = 0; t < Steps; ++t)
    {
        MemoryAccess(Cpu, &Cpu->OpCodes[BS % 3]);
        ExecuteOpCode(Cpu, &Cpu->OpCodes[(BS + 1) % 3]);
        opcode *OpCode = &Cpu->OpCodes[(BS + 2) % 3];
        *OpCode = {};
        DecodeOpcode(Cpu, OpCode, Cpu->MachineCode);
        Cpu->MachineCode = InstructionFetch(Cpu);
        Cpu->IAddress = Cpu->pc - 4;
        ++BS;
    }
    Cpu->BaseState = BS;
}

static void __attribute__((constructor))
InitJumpTables()
{
    for (int i = 0; i <= 0x3F; ++i)
    {
        PrimaryJumpTable[i] = ReservedInstructionException;
        SecondaryJumpTable[i] = ReservedInstructionException;
    }

//    PrimaryJumpTable[0x00] = ExecuteSecondary;
    PrimaryJumpTable[0x01] = BranchZero;
    PrimaryJumpTable[0x02] = J;
    PrimaryJumpTable[0x03] = JAL;
    PrimaryJumpTable[0x04] = BEQ;
    PrimaryJumpTable[0x05] = BNE;
    PrimaryJumpTable[0x06] = BLEZ;
    PrimaryJumpTable[0x07] = BGTZ;
    PrimaryJumpTable[0x08] = AddI;
    PrimaryJumpTable[0x09] = AddIU;
    PrimaryJumpTable[0x0A] = SLTI;
    PrimaryJumpTable[0x0B] = SLTIU;
    PrimaryJumpTable[0x0C] = AndI;
    PrimaryJumpTable[0x0D] = OrI;
    PrimaryJumpTable[0x0E] = XOrI;
    PrimaryJumpTable[0x0F] = LUI;
    PrimaryJumpTable[0x10] = COP0;
    PrimaryJumpTable[0x11] = COP1;
    PrimaryJumpTable[0x12] = COP2;
    PrimaryJumpTable[0x13] = COP3;
    PrimaryJumpTable[0x20] = LB;
    PrimaryJumpTable[0x21] = LH;
    //    PrimaryJumpTable[0x22] = LWL;
    PrimaryJumpTable[0x23] = LW;
    PrimaryJumpTable[0x24] = LBU;
    PrimaryJumpTable[0x25] = LHU;
    //    PrimaryJumpTable[0x26] = LWR;
    PrimaryJumpTable[0x28] = SB;
    PrimaryJumpTable[0x29] = SH;
    //    PrimaryJumpTable[0x2A] = SWL;
    PrimaryJumpTable[0x2B] = SW;
    //    PrimaryJumpTable[0x2E] = SWR;
    //    PrimaryJumpTable[0x30] = LWC0;
    //    PrimaryJumpTable[0x31] = LWC1;
    //    PrimaryJumpTable[0x32] = LWC2;
    //    PrimaryJumpTable[0x33] = LWC3;
    //    PrimaryJumpTable[0x38] = SWC0;
    //    PrimaryJumpTable[0x39] = SWC1;
    //    PrimaryJumpTable[0x3A] = SWC2;
    //    PrimaryJumpTable[0x3B] = SWC3;

    SecondaryJumpTable[0x00] = SLL;
    SecondaryJumpTable[0x02] = SRL;
    SecondaryJumpTable[0x03] = SRA;
    SecondaryJumpTable[0x04] = SLLV;
    SecondaryJumpTable[0x06] = SRLV;
    SecondaryJumpTable[0x07] = SRAV;
    SecondaryJumpTable[0x08] = JR;
    SecondaryJumpTable[0x09] = JALR;
    SecondaryJumpTable[0x0C] = SysCall;
    SecondaryJumpTable[0x0D] = Break;
    SecondaryJumpTable[0x10] = MFHI;
    SecondaryJumpTable[0x11] = MTHI;
    SecondaryJumpTable[0x12] = MFLO;
    SecondaryJumpTable[0x13] = MTLO;
    SecondaryJumpTable[0x18] = Mult;
    SecondaryJumpTable[0x19] = MultU;
    SecondaryJumpTable[0x1A] = Div;
    SecondaryJumpTable[0x1B] = DivU;
    SecondaryJumpTable[0x20] = Add;
    SecondaryJumpTable[0x21] = AddU;
    SecondaryJumpTable[0x22] = Sub;
    SecondaryJumpTable[0x23] = SubU;
    SecondaryJumpTable[0x24] = And;
    SecondaryJumpTable[0x25] = Or;
    SecondaryJumpTable[0x26] = XOr;
    SecondaryJumpTable[0x27] = NOr;
    SecondaryJumpTable[0x2A] = SLT;
    SecondaryJumpTable[0x2B] = SLTU;
}

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

inline u64
ReadMemDWord(MIPS_R3000 *Cpu, u64 Address)
{
    u64 Base = Address & 0x1FFFFFFF;
    u8 *VirtualAddress = (u8 *)MapVirtualAddress(Cpu, Address);
    u32 Swap = Cpu->CP0.sr & C0_STATUS_RE;
    u64 Value = 0;
    if (!VirtualAddress)
    {
        for (u32 i = 0; i < Cpu->NumMMR; ++i)
        {
            mmr *MMR = &Cpu->MemMappedRegisters[i];
            if (Base == MMR->Address)
            {
                // TODO read double from two 32-bit consecutive registers
                // NOTE does R4300 support mapping 64 bit registers?
                Value = MMR->RegisterReadFunc(MMR->Object, Address);
                return (Swap ? Value: __builtin_bswap64(Value));
            }
        }

        return Value;
    }
    Value = *((u64 *)VirtualAddress);
    return (Swap ? Value: __builtin_bswap64(Value));
}

inline u32
ReadMemWord(MIPS_R3000 *Cpu, u64 Address)
{
    u32 Base = Address & 0x1FFFFFFF;
    u8 *VirtualAddress = (u8 *)MapVirtualAddress(Cpu, Address);
    u32 Swap = Cpu->CP0.sr & C0_STATUS_RE;
    u32 Value = 0;
    if (!VirtualAddress)
    {
        for (u32 i = 0; i < Cpu->NumMMR; ++i)
        {
            mmr *MMR = &Cpu->MemMappedRegisters[i];
            if (Base == MMR->Address)
            {
                Value = MMR->RegisterReadFunc(MMR->Object, Address);
                return (Swap ? Value: __builtin_bswap32(Value));
            }
        }

        return Value;
    }
    Value = *((u32 *)VirtualAddress);
    return (Swap ? Value: __builtin_bswap32(Value));
}

static void
WriteMemByte(MIPS_R3000 *Cpu, u64 Address, u8 Value)
{
    u32 Base = Address & 0x1FFFFFFF;
    u8 *VirtualAddress = (u8 *)MapVirtualAddress(Cpu, Address);
    if (!VirtualAddress)
    {
        for (u32 i = 0; i < Cpu->NumMMR; ++i)
        {
            mmr *MMR = &Cpu->MemMappedRegisters[i];
            if (Base == MMR->Address)
            {
                
                MMR->RegisterWriteFunc(MMR->Object, Value);
                return;
            }
        }

        return;
    }
    *((u8 *)VirtualAddress) = Value;
}

static void
WriteMemDWord(MIPS_R3000 *Cpu, u64 Address, u64 Value)
{
    u32 Base = Address & 0x1FFFFFFF;
    u8 *VirtualAddress = (u8 *)MapVirtualAddress(Cpu, Address);
    u32 Swap = Cpu->CP0.sr & C0_STATUS_RE;
    if (!VirtualAddress)
    {
        for (u32 i = 0; i < Cpu->NumMMR; ++i)
        {
            mmr *MMR = &Cpu->MemMappedRegisters[i];
            if (Base == MMR->Address)
            {
                // NOTE does R4300 support writing a single 64 bit register?
                MMR->RegisterWriteFunc(MMR->Object, (Swap ? Value: __builtin_bswap64(Value)));
                return;
            }
        }

        return;
    }
    *((u64 *)((u8 *)VirtualAddress)) = (Swap ? Value: __builtin_bswap64(Value));
}

static void
WriteMemWord(MIPS_R3000 *Cpu, u64 Address, u32 Value)
{
    u32 Base = Address & 0x1FFFFFFF;
    u8 *VirtualAddress = (u8 *)MapVirtualAddress(Cpu, Address);
    u32 Swap = Cpu->CP0.sr & C0_STATUS_RE;
    if (!VirtualAddress)
    {
        for (u32 i = 0; i < Cpu->NumMMR; ++i)
        {
            mmr *MMR = &Cpu->MemMappedRegisters[i];
            if (Base == MMR->Address)
            {
                MMR->RegisterWriteFunc(MMR->Object, (Swap ? Value: __builtin_bswap32(Value)));
                return;
            }
        }

        return;
    }
    *((u32 *)((u8 *)VirtualAddress)) = (Swap ? Value: __builtin_bswap32(Value));
}

static void
WriteMemHalfWord(MIPS_R3000 *Cpu, u64 Address, u16 Value)
{
    u32 Base = Address & 0x1FFFFFFF;
    u8 *VirtualAddress = (u8 *)MapVirtualAddress(Cpu, Address);
    u32 Swap = Cpu->CP0.sr & C0_STATUS_RE;
    if (!VirtualAddress)
    {
        for (u32 i = 0; i < Cpu->NumMMR; ++i)
        {
            mmr *MMR = &Cpu->MemMappedRegisters[i];
            if (Base == MMR->Address)
            {
                MMR->RegisterWriteFunc(MMR->Object, (Swap ? Value: __builtin_bswap16(Value)));
                return;
            }
        }

        return;
    }
    *((u16 *)((u8 *)VirtualAddress)) = (Swap ? Value: __builtin_bswap16(Value));
}

static void
C0ExecuteOperation(Coprocessor *Cp, u32 FunctionCode);

MIPS_R3000::
MIPS_R3000()
{
    CP0.ExecuteOperation = C0ExecuteOperation;
    NumMMR = 0;
    NumMMM = 0;
}

//Exceptions
inline void
C0ExceptionPushSRBits(Coprocessor *CP0)
{
    u64 SR = CP0->sr;
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
    u64 SR = CP0->sr;
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
C0GenerateException(MIPS_R3000 *Cpu, u8 Cause, u64 EPC)
{
    if (Cpu->CP0.sr & C0_STATUS_IEc)
    {
        Cpu->CP0.cause = (Cause << 2) & C0_CAUSE_MASK;
        Cpu->CP0.epc = EPC;
        C0ExceptionPushSRBits(&Cpu->CP0);
        Cpu->CP0.sr &= ~C0_STATUS_KUc;
        Cpu->CP0.sr &= ~C0_STATUS_IEc;
        Cpu->pc = GNRAL_VECTOR;
    }
}

static void
C0GenerateSoftwareException(MIPS_R3000 *Cpu, u8 Cause, u64 EPC)
{
    Cpu->CP0.cause = (Cause << 2) & C0_CAUSE_MASK;
    Cpu->CP0.epc = EPC;
    C0ExceptionPushSRBits(&Cpu->CP0);
    Cpu->CP0.sr &= ~C0_STATUS_KUc;
    Cpu->CP0.sr &= ~C0_STATUS_IEc;
    Cpu->pc = GNRAL_VECTOR;
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
ReservedInstructionException(MIPS_R3000 *Cpu, opcode *Op, u32 Data)
{
    C0GenerateException(Cpu, C0_CAUSE_RI, Op->CurrentAddress);
}

static void
SysCall(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
//    u32 Immediate = (Data & COMMENT20_MASK) >> 6;
    C0GenerateSoftwareException(Cpu, C0_CAUSE_SYSCALL, OpCode->CurrentAddress);
}

static void
Break(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
//    OpCode->Immediate = (Data & COMMENT20_MASK) >> 6;
    C0GenerateException(Cpu, C0_CAUSE_BKPT, OpCode->CurrentAddress);
}

typedef void (*jt_func)(MIPS_R3000 *, opcode *, u32 Data);

//Arithmetic
static void
AddU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] + Cpu->GPR[rt];
    }
}

static void
AddIU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->GPR[rt] = (u32)((Cpu->GPR[rs] & 0xFFFFFFFF) + Immediate);
    }
}

static void
DAddIU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
        Cpu->GPR[rt] = Cpu->GPR[rs] + Immediate;
    }
}

static void
SubU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] - Cpu->GPR[rt];
    }
}

static void
Add(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] + Cpu->GPR[rt];
    }
    // TODO overflow trap
}

static void
AddI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->GPR[rt] = (u32)((Cpu->GPR[rs] & 0xFFFFFFFF) + Immediate);
    }
    // TODO overflow trap
}

static void
DAddI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
        Cpu->GPR[rt] = Cpu->GPR[rs] + Immediate;
    }
    // TODO overflow trap
}

static void
Sub(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] - Cpu->GPR[rt];
    }
    // TODO overflow trap
}

//HI:LO operations
static void
MFHI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;
    Cpu->GPR[rd] = Cpu->hi;
}

static void
MFLO(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;
    Cpu->GPR[rd] = Cpu->lo;
}

static void
MTHI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    Cpu->hi = rs;
}

static void
MTLO(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    Cpu->lo = rs;
}

static void
Mult(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    s64 Result = (s64)Cpu->GPR[rs] * (s64)Cpu->GPR[rt];
    Cpu->hi = (Result >> 32) & 0xFFFFFFFF;
    Cpu->lo = Result & 0xFFFFFFFF;
}

static void
MultU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    u64 Result = (u64)Cpu->GPR[rs] * (u64)Cpu->GPR[rt];
    Cpu->hi = Result >> 32;
    Cpu->lo = Result & 0xFFFFFFFF;
}

static void
Div(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    s64 Left = Cpu->GPR[rs];
    s64 Right = Cpu->GPR[rt];
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
    if (Right == -1 && (u64)Left == 0x8000000000000000)
    {
        Cpu->hi = 0;
        Cpu->lo = 0x8000000000000000;
        return;
    }
    Cpu->lo = Left / Right;
    Cpu->hi = Left % Right;
}

static void
DivU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    u64 Left = Cpu->GPR[rs];
    u64 Right = Cpu->GPR[rt];
    if (!Right)
    {
        Cpu->hi = Left;
        Cpu->lo = 0xFFFFFFFFFFFFFFFF;
        return;
    }
    Cpu->lo = Left / Right;
    Cpu->hi = Left % Right;
}

//Store
static void
SW(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_WORD | MEM_ACCESS_WRITE;
}

static void
SD(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_DWORD | MEM_ACCESS_WRITE;
}

static void
SDL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_DWORD | MEM_ACCESS_WRITE | MEM_ACCESS_HIGH;
}

static void
SDR(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_DWORD | MEM_ACCESS_WRITE | MEM_ACCESS_LOW;
}

static void
SWL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_WORD | MEM_ACCESS_WRITE | MEM_ACCESS_HIGH;
}

static void
SWR(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_WORD | MEM_ACCESS_WRITE | MEM_ACCESS_LOW;
}

static void
SH(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_HALF | MEM_ACCESS_WRITE;
}

static void
SB(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
    OpCode->MemAccessValue = Cpu->GPR[rt];
    OpCode->MemAccessMode = MEM_ACCESS_BYTE | MEM_ACCESS_WRITE;
}

//Load
static void
LUI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 Immediate = (Data & IMM16_MASK) >> 0;
        Cpu->GPR[rt] = Immediate << 16;
    }
}

static void
LW(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_WORD | MEM_ACCESS_SIGNED;
    }
}

static void
LWU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_WORD;
    }
}

static void
LWL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_WORD | MEM_ACCESS_SIGNED | MEM_ACCESS_HIGH;
    }
}

static void
LWR(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_WORD | MEM_ACCESS_SIGNED | MEM_ACCESS_LOW;
    }
}

static void
LD(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_DWORD;
    }
}

static void
LDL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_DWORD | MEM_ACCESS_HIGH;
    }
}

static void
LDR(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_DWORD | MEM_ACCESS_LOW;
    }
}

static void
LBU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_BYTE;
    }
}

static void
LHU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_HALF;
    }
}

static void
LB(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_BYTE | MEM_ACCESS_SIGNED;
    }
}

static void
LH(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;
    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        OpCode->MemAccessAddress = Cpu->GPR[rs] + Immediate;
        OpCode->MemAccessValue = rt;
        OpCode->MemAccessMode = MEM_ACCESS_READ | MEM_ACCESS_HALF | MEM_ACCESS_SIGNED;
    }
}


// Jump/Call
static void
J(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    Cpu->pc = (Cpu->pc & 0xF0000000) + (Immediate * 4);
}

static void
JAL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
    Cpu->ra = OpCode->CurrentAddress + 8;
    Cpu->pc = (Cpu->pc & 0xF0000000) + (Immediate * 4);
}

static void
JR(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    Cpu->pc = Cpu->GPR[rs];
}

static void
JALR(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rd = (Data & REG_RD_MASK) >> 11;
    Cpu->pc = Cpu->GPR[rs];
    if (rd) Cpu->GPR[rd] = OpCode->CurrentAddress + 8;
}

static void
BranchZero(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    u64 Immediate = SignExtend16To64((Data & IMM16_MASK));

    //bltz, bgez, bltzal, bgezal
    u64 Address = OpCode->CurrentAddress + 4 + Immediate * 4;
    s64 Check = Cpu->GPR[rs];

    if (rt & 0b00001)
    {
        //bgez
        if (Check >= 0)
        {
            if (rt & 0b10000)
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
            if (rt & 0b10000)
            {
                Cpu->ra = OpCode->CurrentAddress + 8;
            }
            Cpu->pc = Address;
        }
    }
}

static void
BEQ(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;
    
    if (Cpu->GPR[rs] == Cpu->GPR[rt])
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BEQL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (Cpu->GPR[rs] == Cpu->GPR[rt])
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    } else {
        Cpu->SkipExecute = true;
    }
}

static void
BNE(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (Cpu->GPR[rs] != Cpu->GPR[rt])
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BNEL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (Cpu->GPR[rs] != Cpu->GPR[rt])
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    } else {
        Cpu->SkipExecute = true;
    }
}

static void
BLEZ(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;

    if (Cpu->GPR[rs] <= 0)
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BLEZL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;

    if (Cpu->GPR[rs] <= 0)
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    } else {
        Cpu->SkipExecute = true;
    }
}

static void
BGTZ(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;

    if (Cpu->GPR[rs] > 0)
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    }
}

static void
BGTZL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;

    if (Cpu->GPR[rs] > 0)
    {
        u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
        Cpu->pc = OpCode->CurrentAddress + 4 + Immediate * 4;
    } else {
        Cpu->SkipExecute = true;
    }
}

//Logical
static void
AndI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = Data & IMM16_MASK;
        Cpu->GPR[rt] = Cpu->GPR[rs] & Immediate;
    }
}

static void
OrI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = Data & IMM16_MASK;
        Cpu->GPR[rt] = Cpu->GPR[rs] | Immediate;
    }
}

static void
And(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] & Cpu->GPR[rt];
    }
}

static void
Or(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] | Cpu->GPR[rt];
    }
}

static void
XOr(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rs] ^ Cpu->GPR[rt];
    }
}
static void
NOr(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = 0xFFFFFFFF ^ (Cpu->GPR[rs] | Cpu->GPR[rt]);
    }
}

static void
XOrI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = Data & IMM16_MASK;
        Cpu->GPR[rt] = Cpu->GPR[rs] ^ Immediate;
    }
}

//shifts
static void
SLLV(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rt] << (Cpu->GPR[rs] & 0x1F);
    }
}

static void
SRLV(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rt] >> (Cpu->GPR[rs] & 0x1F);
    }
}

static void
SRAV(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = ((s32)Cpu->GPR[rt]) >> (Cpu->GPR[rs] & 0x1F);
    }
}

static void
SLL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 Immediate = (Data & IMM5_MASK) >> 6;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rt] << Immediate;
    }
}

static void
SRL(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 Immediate = (Data & IMM5_MASK) >> 6;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = Cpu->GPR[rt] >> Immediate;
    }
}

static void
SRA(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 Immediate = (Data & IMM5_MASK) >> 6;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = ((s32)Cpu->GPR[rt]) >> Immediate;
    }
}

// comparison
static void
SLT(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = ( ((s32)Cpu->GPR[rs] < (s32)Cpu->GPR[rt]) ? 1 : 0);
    }
}

static void
SLTU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (rd)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 rt = (Data & REG_RT_MASK) >> 16;
        Cpu->GPR[rd] = ( (Cpu->GPR[rs] < Cpu->GPR[rt]) ? 1 : 0);
    }
}

static void
SLTI(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        Cpu->GPR[rt] = ( ((s32)Cpu->GPR[rs] < (s32)Immediate) ? 1 : 0);
    }
}

static void
SLTIU(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rt = (Data & REG_RT_MASK) >> 16;

    if (rt)
    {
        u32 rs = (Data & REG_RS_MASK) >> 21;
        u32 Immediate = (Data & IMM16_MASK) >> 0;
        Cpu->GPR[rt] = ( (Cpu->GPR[rs] < Immediate) ? 1 : 0);
    }
}

// coprocessor ops
static void
COP0(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;
    u32 rd = (Data & REG_RD_MASK) >> 11;

    Coprocessor *CP0 = &Cpu->CP0;

    if (rs < 0b001000 && rs > 0b00010)
    {
        CP0->registers[rd + (rs & 0b00010 ? 32 : 0)] = Cpu->GPR[rt];
    }
    else if (rs < 0b10000)
    {
        if (rs < 0b00100)
        {
            if (rt) Cpu->GPR[rt] = CP0->registers[rd + (rs & 0b00010 ? 32 : 0)];
        }
        else
        {
            u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
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
COP1(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    // PSX missing cop1
    // TODO exceptions
}

static void
COP2(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;
    u32 rd = (Data & REG_RD_MASK) >> 11;

    Coprocessor *CP2 = Cpu->CP2;
    if (rs < 0b001000 && rs > 0b00010)
    {
        CP2->registers[rd + (rs & 0b00010 ? 32 : 0)] = Cpu->GPR[rt];
    }
    else if (rs < 0b10000)
    {
        if (rs < 0b00100)
        {
            if (rt) Cpu->GPR[rt] = CP2->registers[rd + (rs & 0b00010 ? 32 : 0)];
        }
        else
        {
            u64 Immediate = SignExtend16To64((Data & IMM16_MASK));
            if (rt)
            {
                if (Cpu->CP0.sr & C0_STATUS_CU2)
                {
                    Cpu->pc = OpCode->CurrentAddress + Immediate;
                }
            }
            else
            {
                if ((Cpu->CP0.sr & C0_STATUS_CU2) == 0)
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
COP3(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data)
{
    // PSX missing cop3
    // TODO exceptions
}

inline u32
InstructionFetch(MIPS_R3000 *Cpu)
{
    u32 Result = ReadMemWord(Cpu, Cpu->pc);
    Cpu->pc += 4;
    return Result;
}

inline void
MemoryAccess(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u64 Address = OpCode->MemAccessAddress;
    u64 Value = OpCode->MemAccessValue;
    u32 MemAccessMode = OpCode->MemAccessMode;

    if (MemAccessMode & MEM_ACCESS_WRITE)
    {
        if (MemAccessMode & MEM_ACCESS_BYTE)
        {
            WriteMemByte(Cpu, Address, (u8)Value);
        }

        else if (MemAccessMode & MEM_ACCESS_HALF)
        {
            WriteMemHalfWord(Cpu, Address, (u16)Value);
        }

        else if (MemAccessMode & MEM_ACCESS_WORD)
        {
            if (MemAccessMode & MEM_ACCESS_HIGH)
            {
                Value = (Value & 0xFFFF0000) | (ReadMemWordRaw(Cpu, Address) & 0x0000FFFF);
            }
            else if (MemAccessMode & MEM_ACCESS_LOW)
            {
                Value = (Value & 0x0000FFFF) | (ReadMemWordRaw(Cpu, Address) & 0xFFFF0000);
            }
            WriteMemWord(Cpu, Address, (u32)Value);
        }

        else if (MemAccessMode & MEM_ACCESS_DWORD)
        {
            if (MemAccessMode & MEM_ACCESS_HIGH)
            {
                Value = (Value & 0xFFFFFFFF00000000) | (ReadMemDWordRaw(Cpu, Address) & 0x00000000FFFFFFFF);
            }
            else if (MemAccessMode & MEM_ACCESS_LOW)
            {
                Value = (Value & 0x00000000FFFFFFFF) | (ReadMemDWordRaw(Cpu, Address) & 0xFFFFFFFF00000000);
            }
            WriteMemDWord(Cpu, Address, Value);
        }
    }
    else if (MemAccessMode & MEM_ACCESS_READ)
    {
        if (Value)
        {
            u32 Register = (u32)Value;
            u32 Signed = MemAccessMode & MEM_ACCESS_SIGNED;
            Value = ReadMemDWord(Cpu, Address);
            if (MemAccessMode & MEM_ACCESS_BYTE)
            {
                Value &= 0xFF;
                if (Signed)
                {
                    Value = SignExtend8To64(Value);
                }
            }

            else if (MemAccessMode & MEM_ACCESS_HALF)
            {
                Value &= 0xFFFF;
                if (Signed)
                {
                    Value = SignExtend16To64(Value);
                }
            }

            else if (MemAccessMode & MEM_ACCESS_WORD)
            {
                Value &= 0xFFFFFFFF;
                if (Signed)
                {
                    Value = SignExtend32To64(Value);
                }
                if (MemAccessMode & MEM_ACCESS_HIGH)
                {
                    Value = SignExtend32To64((Cpu->GPR[Register] & 0x0000FFFF) | (Value & 0xFFFF0000));
                }
                else if (MemAccessMode & MEM_ACCESS_LOW)
                {
                    Value = SignExtend32To64((Cpu->GPR[Register] & 0xFFFF0000) | (Value & 0x0000FFFF));
                }
            }

            else if (MemAccessMode & MEM_ACCESS_DWORD)
            {
                if (MemAccessMode & MEM_ACCESS_HIGH)
                {
                    Value = (Cpu->GPR[Register] & 0xFFFFFFFF) | (Value & 0xFFFFFFFF00000000);
                }
                else if (MemAccessMode & MEM_ACCESS_LOW)
                {
                    Value = (Cpu->GPR[Register] & 0xFFFFFFFF00000000) | (Value & 0xFFFFFFFF);
                }
            }
            Cpu->GPR[Register] = Value;
        }
    }
}

void
MapRegister(MIPS_R3000 *Cpu, mmr MMR)
{
    Cpu->MemMappedRegisters[Cpu->NumMMR] = MMR;
    ++Cpu->NumMMR;
}

void
MapMemoryRegion(MIPS_R3000 *Cpu, mmm MMM)
{
    Cpu->MemMappedMemRegions[Cpu->NumMMM] = MMM;
    ++Cpu->NumMMM;
}

void
StepCpu(MIPS_R3000 *Cpu, u32 Steps)
{
    void *FJTPrimary[0x40] =
    {
        &&_ReservedInstructionException,
        &&_BranchZero,
        &&_Jump,
        &&_JumpAL,
        &&_BEQ,
        &&_BNE,
        &&_BLEZ,
        &&_BGTZ,
        &&_AddI,
        &&_AddIU,
        &&_SLTI,
        &&_SLTIU,
        &&_AndI,
        &&_OrI,
        &&_XOrI,
        &&_LUI,
        &&_COP0,
        &&_COP1,
        &&_COP2,
        &&_COP3,
        &&_BEQL,
        &&_BNEL,
        &&_BLEZL,
        &&_BGTZL,
        &&_DAddI,
        &&_DAddIU,
        &&_LDL,
        &&_LDR,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_LB,
        &&_LH,
        &&_LWL,
        &&_LW,
        &&_LBU,
        &&_LHU,
        &&_LWR,
        &&_LWU,
        &&_SB,
        &&_SH,
        &&_SWL,
        &&_SW,
        &&_SDL,
        &&_SDR,
        &&_SWR,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException, // &&_LWC0,
        &&_ReservedInstructionException, // &&_LWC1,
        &&_ReservedInstructionException, // &&_LWC2,
        &&_ReservedInstructionException, // &&_LWC3,
        &&_ReservedInstructionException, // &&_LLD
        &&_ReservedInstructionException, // &&_LDC1
        &&_ReservedInstructionException, // &&_LDC2
        &&_LD,
        &&_ReservedInstructionException, //&&_SWC0,
        &&_ReservedInstructionException, //&&_SWC1,
        &&_ReservedInstructionException, //&&_SWC2,
        &&_ReservedInstructionException, //&&_SWC3,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_SD,
    };

    void *FJTSecondary[0x40] =
    {
        &&_SLL,
        &&_ReservedInstructionException,
        &&_SRL,
        &&_SRA,
        &&_SLLV,
        &&_ReservedInstructionException,
        &&_SRLV,
        &&_SRAV,
        &&_JumpR,
        &&_JumpALR,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_Syscall,
        &&_Break,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_MFHI,
        &&_MTHI,
        &&_MFLO,
        &&_MTLO,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_Mult,
        &&_MultU,
        &&_Div,
        &&_DivU,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_Add,
        &&_AddU,
        &&_Sub,
        &&_SubU,
        &&_And,
        &&_Or,
        &&_XOr,
        &&_NOr,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_SLT,
        &&_SLTU,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
        &&_ReservedInstructionException,
    };

    opcode *OpCodes = Cpu->OpCodes;
    u32 BS = Cpu->BaseState;
    void *NextJump = Cpu->NextJump;
    u32 NextData = Cpu->NextData;

    if (NextJump) goto *NextJump;

#define NEXT(Instruction) \
    { \
    if (!Steps) goto _ExitThread; \
    opcode *OpCodeMemAccess = &OpCodes[BS % 2]; \
    if (OpCodeMemAccess->MemAccessMode) \
    { \
        MemoryAccess(Cpu, OpCodeMemAccess); \
    } \
    OpCodeMemAccess->CurrentAddress = Cpu->pc; \
    OpCodeMemAccess->MemAccessMode = MEM_ACCESS_NONE; \
    u32 TempData = InstructionFetch(Cpu); \
    if (!Cpu->SkipExecute) { \
        Instruction(Cpu, &OpCodes[(BS + 1) % 2], NextData); \
    } else { \
        Cpu->SkipExecute = 0; \
    } \
    NextData = TempData; \
    if ((NextData & PRIMARY_OP_MASK) >> 26) \
    { \
        NextJump = FJTPrimary[(NextData & PRIMARY_OP_MASK) >> 26]; \
    } \
    else \
    { \
        u32 Select1 = (NextData & SECONDARY_OP_MASK); \
        NextJump = FJTSecondary[Select1]; \
    } \
    ++BS; \
    --Steps; \
    goto *NextJump; \
    }


_ReservedInstructionException:
    NEXT(ReservedInstructionException);
_BranchZero:
    NEXT(BranchZero);
_Jump:
    NEXT(J);
_JumpAL:
    NEXT(JAL);
_BEQ:
    NEXT(BEQ);
_BNE:
    NEXT(BNE);
_BLEZ:
    NEXT(BLEZ);
_BGTZ:
    NEXT(BGTZ);
_AddI:
    NEXT(AddI);
_AddIU:
    NEXT(AddIU);
_SLTI:
    NEXT(SLTI);
_SLTIU:
    NEXT(SLTIU);
_AndI:
    NEXT(AndI);
_OrI:
    NEXT(OrI);
_XOrI:
    NEXT(XOrI);
_LUI:
    NEXT(LUI);
_COP0:
    NEXT(COP0);
_COP1:
    NEXT(COP1);
_COP2:
    NEXT(COP2);
_COP3:
    NEXT(COP3);
_BEQL:
    NEXT(BEQL);
_BNEL:
    NEXT(BNEL);
_BLEZL:
    NEXT(BLEZL);
_BGTZL:
    NEXT(BGTZL);
_DAddI:
    NEXT(DAddI);
_DAddIU:
    NEXT(DAddIU);
_LDL:
    NEXT(LDL);
_LDR:
    NEXT(LDR);
_LB:
    NEXT(LB);
_LH:
    NEXT(LH);
_LWL:
    NEXT(LWL);
_LW:
    NEXT(LW);
_LBU:
    NEXT(LBU);
_LHU:
    NEXT(LHU);
_LWR:
    NEXT(LWR);
_LWU:
    NEXT(LWU);
_SB:
    NEXT(SB);
_SH:
    NEXT(SH);
_SWL:
    NEXT(SWL);
_SW:
    NEXT(SW);
_SDL:
    NEXT(SDL);
_SDR:
    NEXT(SDR);
_SWR:
    NEXT(SWR);
_LD:
    NEXT(LD);
_SD:
    NEXT(SD);

_SLL:
    NEXT(SLL);
_SRL:
    NEXT(SRL);
_SRA:
    NEXT(SRA);
_SLLV:
    NEXT(SLLV);
_SRLV:
    NEXT(SRLV);
_SRAV:
    NEXT(SRAV);
_JumpR:
    NEXT(JR);
_JumpALR:
    NEXT(JALR);
_Syscall:
    NEXT(SysCall);
_Break:
    NEXT(Break);
_MFHI:
    NEXT(MFHI);
_MTHI:
    NEXT(MTHI);
_MFLO:
    NEXT(MFLO);
_MTLO:
    NEXT(MTLO);
_Mult:
    NEXT(Mult);
_MultU:
    NEXT(MultU);
_Div:
    NEXT(Div);
_DivU:
    NEXT(DivU);
_Add:
    NEXT(Add);
_AddU:
    NEXT(AddU);
_Sub:
    NEXT(Sub);
_SubU:
    NEXT(SubU);
_And:
    NEXT(And);
_Or:
    NEXT(Or);
_XOr:
    NEXT(XOr);
_NOr:
    NEXT(NOr);
_SLT:
    NEXT(SLT);
_SLTU:
    NEXT(SLTU);

_ExitThread:
    Cpu->NextJump = NextJump;
    Cpu->NextData = NextData;
    Cpu->BaseState = BS;
}

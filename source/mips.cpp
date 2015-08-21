#include <3ds.h>
#include <cstdio>
#include "mips.h"
#include "debugger.h"



inline u32
ReadMemWord(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u32 *)((u8 *)Cpu->Memory + Base));
}

inline u8
ReadMemByte(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u8 *)Cpu->Memory + Base);
}

inline u16
ReadMemHalfWord(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u16 *)((u8 *)Cpu->Memory + Base));
}

inline void
WriteMemByte(MIPS_R3000 *Cpu, u32 Address, u8 value)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        if (Base == Cpu->MemMappedRegisters[i].Address)
        {
            Cpu->MemMappedRegisters[i].RegisterFunc(Cpu->MemMappedRegisters[i].Object, value);
            return;
        }
    }
    *((u8 *)Cpu->Memory + Base) = value;
}

inline void
WriteMemWord(MIPS_R3000 *Cpu, u32 Address, u32 value)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        if (Base == Cpu->MemMappedRegisters[i].Address)
        {
             Cpu->MemMappedRegisters[i].RegisterFunc(Cpu->MemMappedRegisters[i].Object, value);
            return;
        }
    }
    *((u32 *)((u8 *)Cpu->Memory + Base)) = value;
}

inline void
WriteMemHalfWord(MIPS_R3000 *Cpu, u32 Address, u16 value)
{
    u32 Base = Address & 0x00FFFFFF;
    for (u32 i = 0; i < Cpu->NumMMR; ++i)
    {
        if (Base == Cpu->MemMappedRegisters[i].Address)
        {
             Cpu->MemMappedRegisters[i].RegisterFunc(Cpu->MemMappedRegisters[i].Object, value);
            return;
        }
    }
    *((u16 *)((u8 *)Cpu->Memory + Base)) = value;
}

static void
C0ExecuteOperation(Coprocessor *Cp, u32 FunctionCode)
{

}

MIPS_R3000::
MIPS_R3000()
{
    CP0.ExecuteOperation = C0ExecuteOperation;
}

void
DumpState(MIPS_R3000 *Cpu)
{
    printf("\x1b[0;0H");
    for (int i = 0; i < 32; ++i)
    {
        printf("R%02d: 0x%08lX  ", i, Cpu->registers[i]);
        if (i % 2 == 1) printf("\n");
    }
    printf("RA : 0x%08lX\n", Cpu->ra);
    printf("PC : 0x%08lX\n", Cpu->pc);
}

//Exceptions
static void
C0GenerateException(MIPS_R3000 *Cpu, u8 Cause, u32 EPC)
{
    Cpu->CP0.cause = (Cause << 10) & C0_CAUSE_MASK;
    Cpu->CP0.epc = EPC;
    Cpu->CP0.sr |= C0_STATUS_KUc;
    Cpu->pc = GNRAL_VECTOR;
}

static void
ReservedInstructionException(MIPS_R3000 *Cpu, opcode *Op)
{
    // TODO exceptions & coprocessor0
//    DumpState(Cpu, Op);
    DebuggerPrint(__func__);
}


typedef void (*jt_func)(MIPS_R3000 *, opcode *);

static jt_func PrimaryJumpTable[0x40];
static jt_func SecondaryJumpTable[0x40];

//Arithmetic
static void
AddU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->RightValue;
}

static void
AddIU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
}

static void
SubU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue - OpCode->RightValue;
}

static void
Add(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->RightValue;
    // TODO overflow trap
}

static void
AddI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    // TODO overflow trap
}

static void
Sub(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue - OpCode->RightValue;
    // TODO overflow trap
}

//Store
static void
SW(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_WORD;
}

static void
SH(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_HALF;
}

static void
SB(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_BYTE;
}

//Load
static void
LUI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->Immediate << 16;
}

static void
LW(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_WORD;
}

static void
LBU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_BYTE;
}

static void
LHU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_HALF;
}

static void
LB(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_BYTE;
    OpCode->FunctionSelect = 1;
}

static void
LH(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_HALF;
    OpCode->FunctionSelect = 1;
}


// Jump/Call
static void
J(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = (Cpu->pc & 0xF0000000) + (OpCode->Immediate * 4);
}

static void
JAL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->RADestinationRegister = REG_INDEX_RA;
    OpCode->Result = (Cpu->pc & 0xF0000000) + (OpCode->Immediate * 4);
}

static void
JR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue;
}

static void
JALR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue;
}

static void
BranchZero(MIPS_R3000 *Cpu, opcode *OpCode)
{
    //bltz, bgez, bltzal, bgezal
    u8 type = OpCode->RightValue;
    OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
    u32 Check = OpCode->LeftValue;

    if (type & 0b00001)
    {
        //bgez
        if (Check >= 0)
        {
            if (type & 0b10000)
            {
                OpCode->RADestinationRegister = REG_INDEX_RA;
            }
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
    }
    else
    {
        //bltz
        if (Check < 0)
        {
            if (type & 0b10000)
            {
                OpCode->RADestinationRegister = REG_INDEX_RA;
            }
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
    }
}

static void
BEQ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue == OpCode->RightValue)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
    }
}

static void
BNE(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue != OpCode->RightValue)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
    }
}

static void
BLEZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue <= 0)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
    }
}

static void
BGTZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue > 0)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
    }
}

//Logical
static void
AndI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue & OpCode->Immediate;
}

static void
OrI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue | (OpCode->Immediate & 0xFFFF);
}

static void
And(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue & OpCode->RightValue;
}

static void
Or(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue | OpCode->RightValue;
}

static void
XOr(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue ^ OpCode->RightValue;
}
static void
NOr(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = 0xFFFFFFFF ^ (OpCode->LeftValue | OpCode->RightValue);
}

static void
XOrI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue ^ (OpCode->Immediate & 0xFFFF);
}

//shifts
static void
SLLV(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->RightValue << (OpCode->LeftValue & 0x1F);
}

static void
SRLV(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->RightValue >> (OpCode->LeftValue & 0x1F);
}

static void
SRAV(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = ((s32)OpCode->RightValue) >> (OpCode->LeftValue & 0x1F);
}

static void
SLL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->RightValue << OpCode->Immediate;
}

static void
SRL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->RightValue >> OpCode->Immediate;
}

static void
SRA(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = ((s32)OpCode->RightValue) >> OpCode->Immediate;
}

// comparison
static void
SLT(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = ((s32)OpCode->LeftValue < (s32)OpCode->RightValue ? 1 : 0);
}

static void
SLTU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = (OpCode->LeftValue < OpCode->RightValue ? 1 : 0);
}

static void
SLTI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = ((s32)OpCode->LeftValue < (s32)OpCode->Immediate ? 1 : 0);
}

static void
SLTIU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = (OpCode->LeftValue < OpCode->Immediate ? 1 : 0);
}

// coprocessor ops
static void
COP0(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->FunctionSelect < 0b10000)
    {
        if (OpCode->FunctionSelect < 0b01000)
        {
            OpCode->Result = OpCode->LeftValue;
        }
        else
        {
            if (OpCode->RightValue)
            {
                if (Cpu->CP0.sr & C0_STATUS_CU0)
                {
                    OpCode->MemAccessType = MEM_ACCESS_BRANCH;
                    OpCode->Result = OpCode->CurrentAddress + OpCode->Immediate;
                }
            }
            else
            {
                if ((Cpu->CP0.sr & C0_STATUS_CU0) == 0)
                {
                    OpCode->MemAccessType = MEM_ACCESS_BRANCH;
                    OpCode->Result = OpCode->CurrentAddress + OpCode->Immediate;
                }
            }
        }
    }
    else
    {
        Cpu->CP0.ExecuteOperation(&Cpu->CP0, OpCode->Immediate);
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
    if (Cpu->CP1)
    {
        if (OpCode->FunctionSelect < 0b10000)
        {
            if (OpCode->FunctionSelect < 0b01000)
            {
                OpCode->Result = OpCode->LeftValue;
            }
            else
            {
                if (OpCode->RightValue)
                {
                    if (Cpu->CP0.sr & C0_STATUS_CU1)
                    {
                        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
                        OpCode->Result = OpCode->CurrentAddress + OpCode->Immediate;
                    }
                }
                else
                {
                    if ((Cpu->CP0.sr & C0_STATUS_CU1) == 0)
                    {
                        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
                        OpCode->Result = OpCode->CurrentAddress + OpCode->Immediate;
                    }
                }
            }
        }
        else
        {
            Cpu->CP1->ExecuteOperation(Cpu->CP1, OpCode->Immediate);
        }
    }
}

static void
COP3(MIPS_R3000 *Cpu, opcode *OpCode)
{
    // PSX missing cop3
    // TODO exceptions
}


void
DecodeOpcode(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data, u32 IAddress)
{
    OpCode->CurrentAddress = IAddress;
    OpCode->Select0 = (Data & PRIMARY_OP_MASK) >> 26;
    OpCode->Select1 = (Data & SECONDARY_OP_MASK) >> 0;
    OpCode->MemAccessType = MEM_ACCESS_NONE;
    OpCode->MemAccessMode = MEM_ACCESS_WORD;
    OpCode->WriteBackMode = WRITE_BACK_CPU;
    OpCode->LeftValue = 0;
    OpCode->RightValue = 0;
    OpCode->Immediate = 0;
    OpCode->Result = 0;
    OpCode->DestinationRegister = 0;
    OpCode->RADestinationRegister = 0;
    OpCode->FunctionSelect = 0;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    u8 rd = (Data & REG_RD_MASK) >> 11;

    if (OpCode->Select0 == 0)
    {
        //shift-imm
        if ((OpCode->Select1 & 0b111100) == 0)
        {
            OpCode->Immediate = (Data & IMM5_MASK) >> 6;
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = rd;
        }
        //shift-reg
        else if ((OpCode->Select1 & 0b111000) == 0)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = rd;
        }
        //jr
        else if (OpCode->Select1 == 0b001000)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
        //jalr
        else if (OpCode->Select1 == 0b001001)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->RADestinationRegister = rd;
            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
        //sys/brk
        else if ((OpCode->Select1 & 0b111110) == 0b001100)
        {
            OpCode->Immediate = (Data & COMMENT20_MASK) >> 6;
        }
        //mfhi/mflo
        else if ((OpCode->Select1 & 0b111101) == 0b010000)
        {
            OpCode->DestinationRegister = rd;
        }
        //mthi/mtlo
        else if ((OpCode->Select1 & 0b111101) == 0b010001)
        {
            OpCode->DestinationRegister = REG_INDEX_HL;
            OpCode->LeftValue = Cpu->registers[rs];
        }
        //mul/div
        else if ((OpCode->Select1 & 0b111100) == 0b011000)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = REG_INDEX_HL;
        }
        //alu-reg
        else if (OpCode->Select1 & 0b100000)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = rd;
        }

    }
    //bltz, bgez, bltzal bgezal
    else if (OpCode->Select0 == 0b000001)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = rt; //rt is used as a function selector here
        OpCode->Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        //destination registers set within function
    }
    //j/jal
    else if ((OpCode->Select0 & 0b111110) == 0b000010)
    {
        OpCode->Immediate = (Data & IMM26_MASK) >> 0;
        OpCode->DestinationRegister = REG_INDEX_PC;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        //RADestinationRegister set within function
    }
    //beq/bne
    else if ((OpCode->Select0 & 0b111110) == 0b000100)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = Cpu->registers[rt];
        OpCode->Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        //destination registers set within function
    }
    //blez/bgtz
    else if ((OpCode->Select0 & 0b111110) == 0b000110)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        //destination registers set within function
    }
    //alu-imm
    else if ((OpCode->Select0 & 0b111000) == 0b001000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        OpCode->DestinationRegister = rt;
    }
    //lui-imm
    else if (OpCode->Select0 == 0b001111)
    {
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->DestinationRegister = rt;
    }
    //load
    else if ((OpCode->Select0 & 0b111000) == 0b100000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->DestinationRegister = rt;
        OpCode->MemAccessType = MEM_ACCESS_READ;
    }
    //store
    else if ((OpCode->Select0 & 0b111000) == 0b101000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = Cpu->registers[rt];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_WRITE;
    }
    // coprocessor main instruction decoding
    else if ((OpCode->Select0 & 0b010000) == 0b010000)
    {
        OpCode->FunctionSelect = rs;
        // mfc, cfc
        if (rs < 0b00100)
        {
            OpCode->DestinationRegister = rt;
            OpCode->LeftValue = Cpu->CP0.registers[rd + (rs & 0b00010 ? 32 : 0)];
        }
        // mtc, ctc
        else if (rs < 0b01000)
        {
            OpCode->WriteBackMode = OpCode->Select0 & 0b111;
            OpCode->DestinationRegister = rd + (rs & 0b00010 ? 32 : 0);
            OpCode->LeftValue = Cpu->registers[rt];
        }
        // BCnF, BCnT
        else if (rs == 0b01000)
        {
            OpCode->RightValue = rt; // used as secondary function select
            OpCode->Immediate = SignExtend16((Data & IMM16_MASK) >> 0);
        }
        else if (rs & 0b10000)
        {
            OpCode->Immediate = (Data & IMM25_MASK) >> 0;
        }
    }
    //lwc
    else if ((OpCode->Select0 & 0b111000) == 0b110000)
    {
        OpCode->WriteBackMode = OpCode->Select0 & 0b111;
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->DestinationRegister = rt;
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_READ;
    }
    //swc
    else if ((OpCode->Select0 & 0b111000) == 0b111000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = Cpu->CP0.registers[rt];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_WRITE;
    }
}

void
InstructionFetch(MIPS_R3000 *Cpu, u32 *Code)
{
    *Code = ReadMemWord(Cpu, Cpu->pc);
    Cpu->pc += 4;
}

static void
ExecuteSecondary(MIPS_R3000 *Cpu, opcode *Op)
{
    SecondaryJumpTable[Op->Select1](Cpu, Op);
}

void
ExecuteOpCode(MIPS_R3000 *Cpu, opcode *OpCode)
{
    PrimaryJumpTable[OpCode->Select0](Cpu, OpCode);
}

void
MemoryAccess(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->MemAccessType & MEM_ACCESS_WRITE)
    {
        if (OpCode->MemAccessMode & MEM_ACCESS_BYTE)
        {
            WriteMemByte(Cpu, OpCode->Result, OpCode->RightValue);
        }

        if (OpCode->MemAccessMode & MEM_ACCESS_HALF)
        {
            WriteMemHalfWord(Cpu, OpCode->Result, OpCode->RightValue);
        }

        if (OpCode->MemAccessMode & MEM_ACCESS_WORD)
        {
            WriteMemWord(Cpu, OpCode->Result, OpCode->RightValue);
        }
        OpCode->DestinationRegister = 0;
    }
    if (OpCode->MemAccessType & MEM_ACCESS_READ)
    {
        if (OpCode->MemAccessMode & MEM_ACCESS_BYTE)
        {
            if (OpCode->FunctionSelect)
            {
                OpCode->Result = SignExtend8(ReadMemByte(Cpu, OpCode->Result));
            }
            else
            {
                OpCode->Result = ReadMemByte(Cpu, OpCode->Result);
            }
        }

        if (OpCode->MemAccessMode & MEM_ACCESS_HALF)
        {
            if (OpCode->FunctionSelect)
            {
                OpCode->Result = SignExtend16(ReadMemHalfWord(Cpu, OpCode->Result));
            }
            else
            {
                OpCode->Result = ReadMemHalfWord(Cpu, OpCode->Result);
            }

        }

        if (OpCode->MemAccessMode & MEM_ACCESS_WORD)
        {
            OpCode->Result = ReadMemWord(Cpu, OpCode->Result);
        }
    }
}

void
WriteBack(MIPS_R3000 *Cpu, opcode *OpCode)
{

    if (OpCode->WriteBackMode == WRITE_BACK_CPU)
    {
        if (OpCode->DestinationRegister) // Never overwrite Zero
        {
            Cpu->registers[OpCode->DestinationRegister] = OpCode->Result;
        }
        if (OpCode->RADestinationRegister)
        {
            Cpu->registers[OpCode->RADestinationRegister] = OpCode->CurrentAddress + 8;
        }
    }
    else if (OpCode->WriteBackMode == WRITE_BACK_C0)
    {
        Cpu->CP0.registers[OpCode->DestinationRegister] = OpCode->Result;
    }
    else if (OpCode->WriteBackMode == WRITE_BACK_C2)
    {
        // TODO GTE
    }
}

void
ExecuteWriteRegisters(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->WriteBackMode == WRITE_BACK_CPU && (OpCode->MemAccessType == MEM_ACCESS_NONE || OpCode->MemAccessType == MEM_ACCESS_BRANCH))
    {
        if (OpCode->DestinationRegister) // Never overwrite Zero
        {
            Cpu->registers[OpCode->DestinationRegister] = OpCode->Result;
        }
        if (OpCode->RADestinationRegister)
        {
            Cpu->registers[OpCode->RADestinationRegister] = OpCode->CurrentAddress + 8;
        }

        OpCode->DestinationRegister = 0;
        OpCode->RADestinationRegister = 0;
    }
}

void
MapRegister(MIPS_R3000 *Cpu, mmr MMR)
{
    Cpu->MemMappedRegisters[Cpu->NumMMR] = MMR;
    Cpu->MemMappedRegisters[Cpu->NumMMR].Address &= 0x00FFFFFF;
    ++Cpu->NumMMR;
}

static void __attribute__((constructor))
InitJumpTables()
{
    for (int i = 0; i < 0x3F; ++i)
    {
        PrimaryJumpTable[i] = ReservedInstructionException;
        SecondaryJumpTable[i] = ReservedInstructionException;
    }

    PrimaryJumpTable[0x00] = ExecuteSecondary;
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
    //    SecondaryJumpTable[0x0C] = SysCall;
    //    SecondaryJumpTable[0x0D] = Break;
    //    SecondaryJumpTable[0x10] = MFHI;
    //    SecondaryJumpTable[0x11] = MTHI;
    //    SecondaryJumpTable[0x12] = MFLO;
    //    SecondaryJumpTable[0x13] = MTLO;
    //    SecondaryJumpTable[0x18] = Mutl;
    //    SecondaryJumpTable[0x19] = MultU;
    //    SecondaryJumpTable[0x1A] = Div;
    //    SecondaryJumpTable[0x1B] = DivU;
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

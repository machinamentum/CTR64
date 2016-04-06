/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <cstdio>
#include <cstdlib>
#include "mips.h"
#include "disasm.h"

static int (*PrintFunction)(const char *, ...) = printf;

static void
UnknownOpCode(disasm_opcode_info *Op)
{
    // TODO exceptions & coprocessor0
    //    DumpState(Cpu, Op);
    PrintFunction("Unknown Op: 0x%02lX:0x%02lX", Op->Select0, Op->Select1);
}


typedef void (*jt_func)(disasm_opcode_info *);

static jt_func PrimaryJumpTable[0x40];
static jt_func SecondaryJumpTable[0x40];
static const char *RNT[34] =
{
    "zr",
    "at",

    "v0",
    "v1",

    "a0",
    "a1",
    "a2",
    "a3",

    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",

    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",

    "k0",
    "k1",

    "gp",
    "sp",
    "fp",

    "ra",
    "pc",

    "hi:lo"
};

static const char *C0RNT[17] =
{
    "r0",
    "r1",
    "r2",
    "bpc",
    "r4",
    "bda",
    "jumpdest",
    "dcic",
    "bva",
    "bdam",
    "r10",
    "bpcm",
    "sr",
    "cause",
    "epc",
    "prid",
};

const char *
DisassemblerGetGPRName(u32 Reg)
{
    if (Reg >= 32)
    {
        return nullptr;
    }
    return RNT[Reg];
}

void
DumpState(MIPS_R3000 *Cpu)
{
    for (int i = 0; i < 32; ++i)
    {
        PrintFunction("%s: 0x%08llX  ", RNT[i], Cpu->GPR[i]);
        if (i % 2 == 1) PrintFunction("\n");
    }
    PrintFunction("PC : 0x%08llX\n", Cpu->pc);
}

static char ScratchString[5];

inline const char *
C0Name(u32 Reg)
{
    if (Reg > C0_PRID)
    {
        snprintf(ScratchString, 4, "r%lud",  Reg);
        return ScratchString;
    }

    return C0RNT[Reg];
}

inline const char *
CNName(u32 Reg)
{
    snprintf(ScratchString, 3, "r%lud",  Reg);
    return ScratchString;
}

void
DumpC0State(Coprocessor *C0)
{
    for (int i = 0; i < 32; ++i)
    {
        PrintFunction("%s: 0x%08llX  ", C0Name(i), C0->registers[i]);
        if (i % 2 == 1) PrintFunction("\n");
    }
}

//Exceptions
static void
SysCall(disasm_opcode_info *OpCode)
{
    PrintFunction("syscall");
    u32 Immediate = (u32)OpCode->Immediate;
    if (Immediate)
    {
        PrintFunction(" 0x%08lX", Immediate);
    }
    PrintFunction("\n");
}

static void
Break(disasm_opcode_info *OpCode)
{
    PrintFunction("break");
    u32 Immediate = (u32)OpCode->Immediate;
    if (Immediate)
    {
        PrintFunction(" 0x%08lX", Immediate);
    }
}

//Arithmetic
static void
AddU(disasm_opcode_info *OpCode)
{
    if (OpCode->RightValue)
    {
        PrintFunction("addu %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
    }
    else
    {
        PrintFunction("move %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue]);
    }

}

static void
AddIU(disasm_opcode_info *OpCode)
{
    PrintFunction("addiu %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
DAddIU(disasm_opcode_info *OpCode)
{
    PrintFunction("daddiu %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
SubU(disasm_opcode_info *OpCode)
{
    PrintFunction("subu %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
Add(disasm_opcode_info *OpCode)
{
    PrintFunction("add %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
AddI(disasm_opcode_info *OpCode)
{
    PrintFunction("addi %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
DAddI(disasm_opcode_info *OpCode)
{
    PrintFunction("daddi %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
Sub(disasm_opcode_info *OpCode)
{
    PrintFunction("sub %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

//HI:LO operations
static void
MFHI(disasm_opcode_info *OpCode)
{
    PrintFunction("mfhi %s", RNT[OpCode->DestinationRegister]);
}

static void
MFLO(disasm_opcode_info *OpCode)
{
    PrintFunction("mflo %s", RNT[OpCode->DestinationRegister]);
}

static void
MTHI(disasm_opcode_info *OpCode)
{
    PrintFunction("mthi %s", RNT[OpCode->LeftValue]);
}

static void
MTLO(disasm_opcode_info *OpCode)
{
    PrintFunction("mtlo %s", RNT[OpCode->LeftValue]);
}

static void
Mult(disasm_opcode_info *OpCode)
{
    PrintFunction("mult %s, %s", RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
MultU(disasm_opcode_info *OpCode)
{
    PrintFunction("multu %s, %s", RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
Div(disasm_opcode_info *OpCode)
{
    PrintFunction("div %s, %s", RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
DivU(disasm_opcode_info *OpCode)
{
    PrintFunction("divu %s, %s", RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

//Store
static void
SW(disasm_opcode_info *OpCode)
{
    PrintFunction("sw %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SWL(disasm_opcode_info *OpCode)
{
    PrintFunction("swl %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SWR(disasm_opcode_info *OpCode)
{
    PrintFunction("swr %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SD(disasm_opcode_info *OpCode)
{
    PrintFunction("sd %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SDL(disasm_opcode_info *OpCode)
{
    PrintFunction("sdl %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SDR(disasm_opcode_info *OpCode)
{
    PrintFunction("sdr %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SH(disasm_opcode_info *OpCode)
{
    PrintFunction("sh %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
SB(disasm_opcode_info *OpCode)
{
    PrintFunction("sb %s, 0x%04X(%s)", RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

//Load
static void
LUI(disasm_opcode_info *OpCode)
{
    PrintFunction("lui %s, 0x%04X", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate);
}

static void
LW(disasm_opcode_info *OpCode)
{
    PrintFunction("lw %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LWL(disasm_opcode_info *OpCode)
{
    PrintFunction("lwl %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LWR(disasm_opcode_info *OpCode)
{
    PrintFunction("lwr %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LWU(disasm_opcode_info *OpCode)
{
    PrintFunction("lwu %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LD(disasm_opcode_info *OpCode)
{
    PrintFunction("ld %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LDL(disasm_opcode_info *OpCode)
{
    PrintFunction("ldl %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LDR(disasm_opcode_info *OpCode)
{
    PrintFunction("ldr %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LBU(disasm_opcode_info *OpCode)
{
    PrintFunction("lbu %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LHU(disasm_opcode_info *OpCode)
{
    PrintFunction("lhu %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LB(disasm_opcode_info *OpCode)
{
    PrintFunction("lb %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
LH(disasm_opcode_info *OpCode)
{
    PrintFunction("lh %s, 0x%04X(%s)", RNT[OpCode->DestinationRegister], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}


// Jump/Call
static void
J(disasm_opcode_info *OpCode)
{
    PrintFunction("j 0x%08lX", ((u32)OpCode->Immediate) * 4);
}

static void
JAL(disasm_opcode_info *OpCode)
{
    PrintFunction("jal 0x%08lX", ((u32)OpCode->Immediate) * 4);
}

static void
JR(disasm_opcode_info *OpCode)
{
    PrintFunction("jr %s", RNT[OpCode->LeftValue]);
}

static void
JALR(disasm_opcode_info *OpCode)
{
    PrintFunction("jalr %s, %s", RNT[OpCode->RADestinationRegister], RNT[OpCode->LeftValue]);
}

static void
BranchZero(disasm_opcode_info *OpCode)
{
    //bltz, bgez, bltzal, bgezal
    u32 type = OpCode->RightValue;
    if (type & 0b00001)
    {
        //bgez
        PrintFunction("bgez");
        if (type & 0b10000)
        {
            PrintFunction("al");
        }
    }
    else
    {
        //bltz
         PrintFunction("bltz");
        if (type & 0b10000)
        {
            PrintFunction("al");
        }
    }
    PrintFunction(" %s, 0x%04X", RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
BEQ(disasm_opcode_info *OpCode)
{
    PrintFunction("beq %s, %s, 0x%08llX", RNT[OpCode->LeftValue], RNT[OpCode->RightValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BEQL(disasm_opcode_info *OpCode)
{
    PrintFunction("beql %s, %s, 0x%08llX", RNT[OpCode->LeftValue], RNT[OpCode->RightValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BNE(disasm_opcode_info *OpCode)
{
    PrintFunction("bne %s, %s, 0x%08llX", RNT[OpCode->LeftValue], RNT[OpCode->RightValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BNEL(disasm_opcode_info *OpCode)
{
    PrintFunction("bnel %s, %s, 0x%08llX", RNT[OpCode->LeftValue], RNT[OpCode->RightValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BLEZ(disasm_opcode_info *OpCode)
{
    PrintFunction("blez %s, 0x%08llX", RNT[OpCode->LeftValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BLEZL(disasm_opcode_info *OpCode)
{
    PrintFunction("blezl %s, 0x%08llX", RNT[OpCode->LeftValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BGTZ(disasm_opcode_info *OpCode)
{
    PrintFunction("bgtz %s, 0x%08llX", RNT[OpCode->LeftValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

static void
BGTZL(disasm_opcode_info *OpCode)
{
    PrintFunction("bgtzl %s, 0x%08llX", RNT[OpCode->LeftValue], OpCode->CurrentAddress + 4 + OpCode->Immediate * 4);
}

//Logical
static void
AndI(disasm_opcode_info *OpCode)
{
    PrintFunction("andi %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
OrI(disasm_opcode_info *OpCode)
{
    PrintFunction("ori %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
And(disasm_opcode_info *OpCode)
{
    PrintFunction("and %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
Or(disasm_opcode_info *OpCode)
{
    PrintFunction("or %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
XOr(disasm_opcode_info *OpCode)
{
    PrintFunction("xor %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}
static void
NOr(disasm_opcode_info *OpCode)
{
    PrintFunction("nor %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
XOrI(disasm_opcode_info *OpCode)
{
    PrintFunction("xori %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

//shifts
static void
SLLV(disasm_opcode_info *OpCode)
{
    PrintFunction("sllv %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], RNT[OpCode->LeftValue]);
}

static void
SRLV(disasm_opcode_info *OpCode)
{
    PrintFunction("srlv %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], RNT[OpCode->LeftValue]);
}

static void
SRAV(disasm_opcode_info *OpCode)
{
    PrintFunction("srav %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], RNT[OpCode->LeftValue]);
}

static void
SLL(disasm_opcode_info *OpCode)
{
    if (OpCode->DestinationRegister | OpCode->RightValue | OpCode->Immediate)
    {
        PrintFunction("sll %s, %s, 0x%02X", RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], (u8)OpCode->Immediate);
    }
    else
    {
        PrintFunction("nop");
    }
}

static void
SRL(disasm_opcode_info *OpCode)
{
    PrintFunction("srl %s, %s, 0x%02X", RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], (u8)OpCode->Immediate);
}

static void
SRA(disasm_opcode_info *OpCode)
{
    PrintFunction("sra %s, %s, 0x%02X", RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], (u8)OpCode->Immediate);
}

// comparison
static void
SLT(disasm_opcode_info *OpCode)
{
    PrintFunction("slt %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
SLTU(disasm_opcode_info *OpCode)
{
    PrintFunction("sltu %s, %s, %s", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
SLTI(disasm_opcode_info *OpCode)
{
    PrintFunction("slti %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}

static void
SLTIU(disasm_opcode_info *OpCode)
{
    PrintFunction("sltiu %s, %s, 0x%04X", RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], (u16)OpCode->Immediate);
}


// coprocessor ops
static void
COP0(disasm_opcode_info *OpCode)
{
    switch (OpCode->FunctionSelect) {
        case 0b00000:
            PrintFunction("mfc0 %s, %s", RNT[OpCode->DestinationRegister], C0Name(OpCode->LeftValue));
            break;
        case 0b00010:
            PrintFunction("cfc0 %s, %s", RNT[OpCode->DestinationRegister], C0Name(OpCode->LeftValue - 32));
            break;
        case 0b00100:
            PrintFunction("mtc0 %s, %s", RNT[OpCode->LeftValue], C0Name(OpCode->DestinationRegister));
            break;
        case 0b00110:
            PrintFunction("ctc0 %s, %s", RNT[OpCode->LeftValue], C0Name(OpCode->DestinationRegister - 32));
            break;
        case 0b01000:
        {
            if (OpCode->RightValue)
            {
                PrintFunction("bc0t 0x%04X", (u16)OpCode->Immediate);
            }
            else
            {
                PrintFunction("bc0f 0x%04X", (u16)OpCode->Immediate);
            }
        } break;
        case 0b10000:
            PrintFunction("cop0 0x%08lX", (u32)OpCode->Immediate);
            break;
    }
}

static void
COP1(disasm_opcode_info *OpCode)
{
    switch (OpCode->FunctionSelect) {
        case 0b00000:
            PrintFunction("mfc1 %s, %s", RNT[OpCode->DestinationRegister], CNName(OpCode->LeftValue));
            break;
        case 0b00010:
            PrintFunction("cfc1 %s, %s", RNT[OpCode->DestinationRegister], CNName(OpCode->LeftValue - 32));
            break;
        case 0b00100:
            PrintFunction("mtc1 %s, %s", RNT[OpCode->LeftValue], CNName(OpCode->DestinationRegister));
            break;
        case 0b00110:
            PrintFunction("ctc1 %s, %s", RNT[OpCode->LeftValue], CNName(OpCode->DestinationRegister - 32));
            break;
        case 0b01000:
        {
            if (OpCode->RightValue)
            {
                PrintFunction("bc1t 0x%04X", (u16)OpCode->Immediate);
            }
            else
            {
                PrintFunction("bc1f 0x%04X", (u16)OpCode->Immediate);
            }
        } break;
        case 0b10000:
            PrintFunction("cop1 0x%08lX", (u32)OpCode->Immediate);
            break;
    }
}

static void
COP2(disasm_opcode_info *OpCode)
{
    switch (OpCode->FunctionSelect) {
        case 0b00000:
            PrintFunction("mfc2 %s, %s", RNT[OpCode->DestinationRegister], CNName(OpCode->LeftValue));
            break;
        case 0b00010:
            PrintFunction("cfc2 %s, %s", RNT[OpCode->DestinationRegister], CNName(OpCode->LeftValue - 32));
            break;
        case 0b00100:
            PrintFunction("mtc2 %s, %s", RNT[OpCode->LeftValue], CNName(OpCode->DestinationRegister));
            break;
        case 0b00110:
            PrintFunction("ctc2 %s, %s", RNT[OpCode->LeftValue], CNName(OpCode->DestinationRegister - 32));
            break;
        case 0b01000:
        {
            if (OpCode->RightValue)
            {
                PrintFunction("bc2t 0x%04X", (u16)OpCode->Immediate);
            }
            else
            {
                PrintFunction("bc2f 0x%04X", (u16)OpCode->Immediate);
            }
        } break;
        case 0b10000:
            PrintFunction("cop2 0x%08lX", (u32)OpCode->Immediate);
            break;
    }
}

static void
COP3(disasm_opcode_info *OpCode)
{
    switch (OpCode->FunctionSelect) {
        case 0b00000:
            PrintFunction("mfc3 %s, %s", RNT[OpCode->DestinationRegister], CNName(OpCode->LeftValue));
            break;
        case 0b00010:
            PrintFunction("cfc3 %s, %s", RNT[OpCode->DestinationRegister], CNName(OpCode->LeftValue - 32));
            break;
        case 0b00100:
            PrintFunction("mtc3 %s, %s", RNT[OpCode->LeftValue], CNName(OpCode->DestinationRegister));
            break;
        case 0b00110:
            PrintFunction("ctc3 %s, %s", RNT[OpCode->LeftValue], CNName(OpCode->DestinationRegister - 32));
            break;
        case 0b01000:
        {
            if (OpCode->RightValue)
            {
                PrintFunction("bc3t 0x%04X", (u16)OpCode->Immediate);
            }
            else
            {
                PrintFunction("bc3f 0x%04X", (u16)OpCode->Immediate);
            }
        } break;
        case 0b10000:
            PrintFunction("cop3 0x%08lX", (u32)OpCode->Immediate);
            break;
    }
}


void
DisassemblerDecodeOpcode(disasm_opcode_info *OpCode, u32 Data, u64 IAddress)
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
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;
    u32 rd = (Data & REG_RD_MASK) >> 11;

    if (OpCode->Select0 == 0)
    {
        //shift-imm
        if ((OpCode->Select1 & 0b111100) == 0)
        {
            OpCode->Immediate = (Data & IMM5_MASK) >> 6;
            OpCode->RightValue = rt;
            OpCode->DestinationRegister = rd;
        }
        //shift-reg
        else if ((OpCode->Select1 & 0b111000) == 0)
        {
            OpCode->LeftValue = rs;
            OpCode->RightValue = rt;
            OpCode->DestinationRegister = rd;
        }
        //jr
        else if (OpCode->Select1 == 0b001000)
        {
            OpCode->LeftValue = rs;
            OpCode->DestinationRegister = REG_INDEX_PC;
//            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
        //jalr
        else if (OpCode->Select1 == 0b001001)
        {
            OpCode->LeftValue = rs;
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->RADestinationRegister = rd;
//            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
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
            OpCode->LeftValue = rs;
        }
        //mul/div
        else if ((OpCode->Select1 & 0b111100) == 0b011000)
        {
            OpCode->LeftValue = rs;
            OpCode->RightValue = rt;
        }
        //alu-reg
        else if (OpCode->Select1 & 0b100000)
        {
            OpCode->LeftValue = rs;
            OpCode->RightValue = rt;
            OpCode->DestinationRegister = rd;
        }

    }
    //bltz, bgez, bltzal bgezal
    else if (OpCode->Select0 == 0b000001)
    {
        OpCode->LeftValue = rs;
        OpCode->FunctionSelect = rt;
        OpCode->Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
        //destination registers set within function
    }
    //j/jal
    else if ((OpCode->Select0 & 0b111110) == 0b000010)
    {
        OpCode->Immediate = (Data & IMM26_MASK) >> 0;
        OpCode->DestinationRegister = REG_INDEX_PC;
//        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        //RADestinationRegister set within function
    }
    //beq/bne
    else if ((OpCode->Select0 & 0b111110) == 0b000100)
    {
        OpCode->LeftValue = rs;
        OpCode->RightValue = rt;
        OpCode->Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
        //destination registers set within function
    }
    //blez/bgtz
    else if ((OpCode->Select0 & 0b111110) == 0b000110)
    {
        OpCode->LeftValue = rs;
        OpCode->Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
        //destination registers set within function
    }
    //alu-imm
    else if ((OpCode->Select0 & 0b111000) == 0b001000)
    {
        OpCode->LeftValue = rs;
        OpCode->Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
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
        OpCode->LeftValue = rs;
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->DestinationRegister = rt;
        OpCode->MemAccessType = MEM_ACCESS_READ;
    }
    //store
    else if ((OpCode->Select0 & 0b111000) == 0b101000)
    {
        OpCode->LeftValue = rs;
        OpCode->RightValue = rt;
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
            OpCode->LeftValue = rd + (rs & 0b00010 ? 32 : 0);
        }
        // mtc, ctc
        else if (rs < 0b01000)
        {
            OpCode->WriteBackMode = OpCode->Select0 & 0b111;
            OpCode->DestinationRegister = rd + (rs & 0b00010 ? 32 : 0);
            OpCode->LeftValue = rt;
        }
        // BCnF, BCnT
        else if (rs == 0b01000)
        {
            OpCode->RightValue = rt; // used as secondary function select
            OpCode->Immediate = SignExtend16To64((Data & IMM16_MASK) >> 0);
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
        OpCode->LeftValue = rs;
        OpCode->DestinationRegister = rt;
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_READ;
    }
    //swc
    else if ((OpCode->Select0 & 0b111000) == 0b111000)
    {
        OpCode->LeftValue = rs;
        OpCode->RightValue = rt;
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_WRITE;
    }
}

static void
PrintSecondary(disasm_opcode_info *Op)
{
    SecondaryJumpTable[Op->Select1](Op);
}

void
DisassemblerPrintOpCode(disasm_opcode_info *OpCode)
{
    PrintFunction("0x%08llX: ", OpCode->CurrentAddress);
    PrimaryJumpTable[OpCode->Select0](OpCode);
    PrintFunction("\n");
}

void
DisassemblerPrintRange(MIPS_R3000 *Cpu, u64 Base, u32 Count, u64 PC)
{
    for (u32 i = 0; i < Count; ++i)
    {
        u32 MachineCode = ReadMemWordRaw(Cpu, Base + (i * 4));
        disasm_opcode_info OpCode;
        PrintFunction("\x1b[0m");
        if (Base + i * 4 == PC)
        {
            PrintFunction("\x1b[32m"); // IF
        }
        if (Base + i * 4 == PC - 4)
        {
            PrintFunction("\x1b[33m"); // DC
        }
        if (Base + i * 4 == PC - 8)
        {
            PrintFunction("\x1b[31m"); // EXE
        }
        if (Base + i * 4 == PC - 12)
        {
            PrintFunction("\x1b[34m"); // MEM
        }
        if (Base + i * 4 == PC - 16)
        {
            PrintFunction("\x1b[36m"); // WB
        }
        DisassemblerDecodeOpcode(&OpCode, MachineCode, Base + i * 4);
        DisassemblerPrintOpCode(&OpCode);
    }
}

void DisassemblerSetPrintFunction(int (*Func)(const char *, ...))
{
    PrintFunction = Func;
}

static void __attribute__((constructor))
InitJumpTables()
{
    for (int i = 0; i <= 0x3F; ++i)
    {
        PrimaryJumpTable[i] = UnknownOpCode;
        SecondaryJumpTable[i] = UnknownOpCode;
    }

    PrimaryJumpTable[0x00] = PrintSecondary;
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
    PrimaryJumpTable[0x14] = BEQL;
    PrimaryJumpTable[0x15] = BNEL;
    PrimaryJumpTable[0x16] = BLEZL;
    PrimaryJumpTable[0x17] = BGTZL;
    PrimaryJumpTable[0x18] = DAddI;
    PrimaryJumpTable[0x19] = DAddIU;
    PrimaryJumpTable[0x1A] = LDL;
    PrimaryJumpTable[0x1B] = LDR;
    PrimaryJumpTable[0x20] = LB;
    PrimaryJumpTable[0x21] = LH;
    PrimaryJumpTable[0x22] = LWL;
    PrimaryJumpTable[0x23] = LW;
    PrimaryJumpTable[0x24] = LBU;
    PrimaryJumpTable[0x25] = LHU;
    PrimaryJumpTable[0x26] = LWR;
    PrimaryJumpTable[0x27] = LWU;
    PrimaryJumpTable[0x28] = SB;
    PrimaryJumpTable[0x29] = SH;
    PrimaryJumpTable[0x2A] = SWL;
    PrimaryJumpTable[0x2B] = SW;
    PrimaryJumpTable[0x2C] = SDL;
    PrimaryJumpTable[0x2D] = SDR;
    PrimaryJumpTable[0x2E] = SWR;
    //    PrimaryJumpTable[0x30] = LWC0;
    //    PrimaryJumpTable[0x31] = LWC1;
    //    PrimaryJumpTable[0x32] = LWC2;
    //    PrimaryJumpTable[0x33] = LWC3;
    PrimaryJumpTable[0x37] = LD;
    //    PrimaryJumpTable[0x38] = SWC0;
    //    PrimaryJumpTable[0x39] = SWC1;
    //    PrimaryJumpTable[0x3A] = SWC2;
    //    PrimaryJumpTable[0x3B] = SWC3;
    PrimaryJumpTable[0x3F] = SD;

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

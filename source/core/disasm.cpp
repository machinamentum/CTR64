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

static const char *Select0Table[0x40] =
{
    "unk_op0",
    "unk_op0",
    "j",
    "jal",
    "beq",
    "bne",
    "blez",
    "bgtz",
    "addi",
    "addiu",
    "slti",
    "sltiu",
    "andi",
    "ori",
    "xori",
    "lui",
    "cop0",
    "cop1",
    "cop2",
    "cop3",
    "beql",
    "bnel",
    "blezl",
    "bgtzl",
    "daddi",
    "daddiu",
    "ldl",
    "ldr",
    "unk_op0",
    "unk_op0",
    "unk_op0",
    "unk_op0",
    "lb",
    "lh",
    "lwl",
    "lw",
    "lbu",
    "lhu",
    "lwr",
    "lwu",
    "sb",
    "sh",
    "swl",
    "sw",
    "sdl",
    "sdr",
    "swr",
    "cache",
    "ll",
    "lwc1",
    "lwc2",
    "lwc3",
    "lld",
    "ldc1",
    "ldc2",
    "ld",
    "sc",
    "swc1",
    "swc2",
    "swc3",
    "scd",
    "sdc1",
    "sdc2",
    "sd"
};

static const char *Select1Table[0x40] =
{
    "sll",
    "unk_op1",
    "srl",
    "sra",
    "sllv",
    "unk_op1",
    "srlv",
    "srav",
    "jr",
    "jalr",
    "unk_op1",
    "unk_op1",
    "syscall",
    "break",
    "unk_op1",
    "sync",
    "mfhi",
    "mthi",
    "mflo",
    "mtlo",
    "dsllv",
    "unk_op1",
    "dsrlv",
    "dsrav",
    "mult",
    "multu",
    "div",
    "divu",
    "dmult",
    "dmultu",
    "ddiv",
    "ddivu",
    "add",
    "addu",
    "sub",
    "subu",
    "and",
    "or",
    "xor",
    "nor",
    "unk_op1",
    "unk_op1",
    "slt",
    "sltu",
    "dadd",
    "daddu",
    "dsub",
    "dsubu",
    "tge",
    "tgeu",
    "tlt",
    "tltu",
    "teq",
    "unk_op1",
    "tne",
    "unk_op1",
    "dsll",
    "unk_op1",
    "dsrl",
    "dsra",
    "dsll32",
    "unk_op1",
    "dsrl32",
    "dsra32"
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
    if (Reg >= C0_PRID)
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

static void
SysCall_Break(disasm_opcode_info *OpCode)
{
    PrintFunction("%s", Select1Table[OpCode->Select1]);
    u32 Imm = (u32)((OpCode->Immediate & COMMENT20_MASK) >> 6);
    if (Imm) PrintFunction(" 0x%05lX", Imm);
}

static void
ALU_REG(disasm_opcode_info *OpCode)
{
    PrintFunction("%s %s, %s, %s", Select1Table[OpCode->Select1], RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
ALU_IMM(disasm_opcode_info *OpCode)
{
    u16 Imm = OpCode->Immediate & IMM16_MASK;
    PrintFunction("%s %s, %s, 0x%04X", Select0Table[OpCode->Select0], RNT[OpCode->RightValue], RNT[OpCode->LeftValue], Imm);
}


static void
MFHILO(disasm_opcode_info *OpCode)
{
    PrintFunction("%s %s", Select1Table[OpCode->Select1], RNT[OpCode->DestinationRegister]);
}

static void
MTHILO(disasm_opcode_info *OpCode)
{

    PrintFunction("%s %s", Select1Table[OpCode->Select1], RNT[OpCode->LeftValue]);
}

static void
Mult_Div(disasm_opcode_info *OpCode)
{
    PrintFunction("%s %s, %s", Select1Table[OpCode->Select1], RNT[OpCode->LeftValue], RNT[OpCode->RightValue]);
}

static void
LUI(disasm_opcode_info *OpCode)
{
    u16 Imm = OpCode->Immediate & IMM16_MASK;
    PrintFunction("%s %s, 0x%04X", Select0Table[OpCode->Select0], RNT[OpCode->RightValue], Imm);
}

static void
Jump_Imm(disasm_opcode_info *OpCode)
{
    PrintFunction("%s 0x%08lX", Select0Table[OpCode->Select0], ((u32)OpCode->Immediate & IMM26_MASK) * 4);
}

static void
JR(disasm_opcode_info *OpCode)
{
    PrintFunction("%s %s", Select1Table[OpCode->Select1], RNT[OpCode->LeftValue]);
}

static void
JALR(disasm_opcode_info *OpCode)
{
    PrintFunction("%s %s, %s", Select1Table[OpCode->Select1], RNT[OpCode->DestinationRegister], RNT[OpCode->LeftValue]);
}

static void
BranchRegCompare(disasm_opcode_info *OpCode)
{
    u16 Imm = OpCode->Immediate & IMM16_MASK;
    PrintFunction("%s %s, %s, 0x%08llX", Select0Table[OpCode->Select0], RNT[OpCode->LeftValue], RNT[OpCode->RightValue], OpCode->CurrentAddress + 4 + SignExtend16To64(Imm) * 4);
}

static void
Shift_Imm(disasm_opcode_info *OpCode)
{
    u8 Imm = (u8)((OpCode->Immediate & IMM5_MASK) >> 6);
    if (OpCode->DestinationRegister | OpCode->RightValue | Imm)
    {
        PrintFunction("%s %s, %s, 0x%02X", Select1Table[OpCode->Select1], RNT[OpCode->DestinationRegister], RNT[OpCode->RightValue], Imm);
    }
    else
    {
        PrintFunction("nop");
    }
}

static void
TRAP_REG(disasm_opcode_info *OpCode)
{
    u16 Imm = (u16)((OpCode->Immediate & CODE10_MASK) >> 6);
    PrintFunction("%s %s, %s, 0x%03X", Select1Table[OpCode->Select1], RNT[OpCode->LeftValue], RNT[OpCode->RightValue], Imm);
}

static void
Store_Load(disasm_opcode_info *OpCode)
{
    PrintFunction("%s %s, 0x%04X(%s)", Select0Table[OpCode->Select0], RNT[OpCode->RightValue], (u16)OpCode->Immediate, RNT[OpCode->LeftValue]);
}

static void
BranchZero(disasm_opcode_info *OpCode)
{
    //bltz, bgez, bltzal, bgezal
    s16 Imm = SignExtend16To64(OpCode->Immediate & IMM16_MASK);
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
    PrintFunction(" %s, 0x%08llX", RNT[OpCode->LeftValue], OpCode->CurrentAddress + 4 + Imm * 4);
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
    OpCode->LeftValue = (Data & REG_RS_MASK) >> 21;
    OpCode->RightValue = (Data & REG_RT_MASK) >> 16;
    OpCode->Immediate = Data;
    OpCode->DestinationRegister = (Data & REG_RD_MASK) >> 11;
    OpCode->FunctionSelect = 0;
    u32 rs = (Data & REG_RS_MASK) >> 21;
    u32 rt = (Data & REG_RT_MASK) >> 16;
    u32 rd = (Data & REG_RD_MASK) >> 11;

    // coprocessor main instruction decoding
    if ((OpCode->Select0 & 0b111111) == 0b010000)
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
    PrimaryJumpTable[0x02] = Jump_Imm;
    PrimaryJumpTable[0x03] = Jump_Imm;
    PrimaryJumpTable[0x04] = BranchRegCompare;
    PrimaryJumpTable[0x05] = BranchRegCompare;
    PrimaryJumpTable[0x06] = BranchRegCompare;
    PrimaryJumpTable[0x07] = BranchRegCompare;
    PrimaryJumpTable[0x08] = ALU_IMM;
    PrimaryJumpTable[0x09] = ALU_IMM;
    PrimaryJumpTable[0x0A] = ALU_IMM;
    PrimaryJumpTable[0x0B] = ALU_IMM;
    PrimaryJumpTable[0x0C] = ALU_IMM;
    PrimaryJumpTable[0x0D] = ALU_IMM;
    PrimaryJumpTable[0x0E] = ALU_IMM;
    PrimaryJumpTable[0x0F] = LUI;
    PrimaryJumpTable[0x10] = COP0;
    PrimaryJumpTable[0x11] = COP1;
    PrimaryJumpTable[0x12] = COP2;
    PrimaryJumpTable[0x13] = COP3;
    PrimaryJumpTable[0x14] = BranchRegCompare;
    PrimaryJumpTable[0x15] = BranchRegCompare;
    PrimaryJumpTable[0x16] = BranchRegCompare;
    PrimaryJumpTable[0x17] = BranchRegCompare;
    PrimaryJumpTable[0x18] = ALU_IMM;
    PrimaryJumpTable[0x19] = ALU_IMM;
    PrimaryJumpTable[0x1A] = Store_Load;
    PrimaryJumpTable[0x1B] = Store_Load;
    PrimaryJumpTable[0x20] = Store_Load;
    PrimaryJumpTable[0x21] = Store_Load;
    PrimaryJumpTable[0x22] = Store_Load;
    PrimaryJumpTable[0x23] = Store_Load;
    PrimaryJumpTable[0x24] = Store_Load;
    PrimaryJumpTable[0x25] = Store_Load;
    PrimaryJumpTable[0x26] = Store_Load;
    PrimaryJumpTable[0x27] = Store_Load;
    PrimaryJumpTable[0x28] = Store_Load;
    PrimaryJumpTable[0x29] = Store_Load;
    PrimaryJumpTable[0x2A] = Store_Load;
    PrimaryJumpTable[0x2B] = Store_Load;
    PrimaryJumpTable[0x2C] = Store_Load;
    PrimaryJumpTable[0x2D] = Store_Load;
    PrimaryJumpTable[0x2E] = Store_Load;
    //    PrimaryJumpTable[0x30] = LWC0;
    //    PrimaryJumpTable[0x31] = LWC1;
    //    PrimaryJumpTable[0x32] = LWC2;
    //    PrimaryJumpTable[0x33] = LWC3;
    PrimaryJumpTable[0x37] = Store_Load;
    //    PrimaryJumpTable[0x38] = SWC0;
    //    PrimaryJumpTable[0x39] = SWC1;
    //    PrimaryJumpTable[0x3A] = SWC2;
    //    PrimaryJumpTable[0x3B] = SWC3;
    PrimaryJumpTable[0x3F] = Store_Load;

    SecondaryJumpTable[0x00] = Shift_Imm;
    SecondaryJumpTable[0x02] = Shift_Imm;
    SecondaryJumpTable[0x03] = Shift_Imm;
    SecondaryJumpTable[0x04] = ALU_REG;
    SecondaryJumpTable[0x06] = ALU_REG;
    SecondaryJumpTable[0x07] = ALU_REG;
    SecondaryJumpTable[0x08] = JR;
    SecondaryJumpTable[0x09] = JALR;
    SecondaryJumpTable[0x0C] = SysCall_Break;
    SecondaryJumpTable[0x0D] = SysCall_Break;
    SecondaryJumpTable[0x10] = MFHILO;
    SecondaryJumpTable[0x11] = MTHILO;
    SecondaryJumpTable[0x12] = MFHILO;
    SecondaryJumpTable[0x13] = MTHILO;
    SecondaryJumpTable[0x18] = Mult_Div;
    SecondaryJumpTable[0x19] = Mult_Div;
    SecondaryJumpTable[0x1A] = Mult_Div;
    SecondaryJumpTable[0x1B] = Mult_Div;
    SecondaryJumpTable[0x20] = ALU_REG;
    SecondaryJumpTable[0x21] = ALU_REG;
    SecondaryJumpTable[0x22] = ALU_REG;
    SecondaryJumpTable[0x23] = ALU_REG;
    SecondaryJumpTable[0x24] = ALU_REG;
    SecondaryJumpTable[0x25] = ALU_REG;
    SecondaryJumpTable[0x26] = ALU_REG;
    SecondaryJumpTable[0x27] = ALU_REG;
    SecondaryJumpTable[0x2A] = ALU_REG;
    SecondaryJumpTable[0x2B] = ALU_REG;
    SecondaryJumpTable[0x2C] = ALU_REG;
    SecondaryJumpTable[0x2D] = ALU_REG;
    SecondaryJumpTable[0x2E] = ALU_REG;
    SecondaryJumpTable[0x2F] = ALU_REG;
    SecondaryJumpTable[0x30] = TRAP_REG;
    SecondaryJumpTable[0x31] = TRAP_REG;
    SecondaryJumpTable[0x32] = TRAP_REG;
    SecondaryJumpTable[0x33] = TRAP_REG;
    SecondaryJumpTable[0x34] = TRAP_REG;
    SecondaryJumpTable[0x36] = TRAP_REG;
}

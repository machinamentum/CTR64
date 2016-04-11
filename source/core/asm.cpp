/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "asm.h"

typedef u32 (*jt_func)(disasm_opcode_info *);

static jt_func PrimaryJumpTable[0x40];
static jt_func SecondaryJumpTable[0x40];

static u32
UnknownOpCode(disasm_opcode_info *OpCode)
{
    return ASM_UNK_OP;
}

static u32
SysCall_Break(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_COMMENT20(OpCode->Immediate) | ASM_SELECT1(OpCode->Select1);
}

static u32
ALU_REG(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->RightValue) | ASM_RD(OpCode->DestinationRegister) | ASM_SELECT1(OpCode->Select1);
}

static u32
ALU_IMM(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(OpCode->Select0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->DestinationRegister) | ASM_IMM16(OpCode->Immediate);
}

static u32
MFHILO(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RD(OpCode->DestinationRegister) | ASM_SELECT1(OpCode->Select1);
}

static u32
MTHILO(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RS(OpCode->LeftValue) | ASM_SELECT1(OpCode->Select1);
}

static u32
Mult_Div(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->RightValue) | ASM_SELECT1(OpCode->Select1);
}

static u32
Store_Load(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(OpCode->Select0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->RightValue) | ASM_IMM16(OpCode->Immediate);
}

static u32
LUI(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(OpCode->Select0) | ASM_RT(OpCode->DestinationRegister) | ASM_IMM16(OpCode->Immediate);
}

static u32
Jump_Imm(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(OpCode->Select0) | ASM_IMM26(OpCode->Immediate);
}

static u32
JR(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RS(OpCode->LeftValue) | ASM_SELECT1(OpCode->Select1);
}

static u32
JALR(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RS(OpCode->LeftValue) | ASM_RD(OpCode->DestinationRegister) | ASM_SELECT1(OpCode->Select1);
}

static u32
BranchZero(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(OpCode->Select0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->RightValue) | ASM_IMM16(OpCode->Immediate);
}

static u32
BranchRegCompare(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(OpCode->Select0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->RightValue) | ASM_IMM16(OpCode->Immediate);
}

static u32
Shift_Imm(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RT(OpCode->RightValue) | ASM_RD(OpCode->DestinationRegister) | ASM_IMM5(OpCode->Immediate) | ASM_SELECT1(OpCode->Select1);
}

static u32
TRAP_REG(disasm_opcode_info *OpCode)
{
    return ASM_SELECT0(0) | ASM_RS(OpCode->LeftValue) | ASM_RT(OpCode->RightValue) | ASM_CODE10(OpCode->Immediate) | ASM_SELECT1(OpCode->Select1);
}

static u32
Coprocessor(disasm_opcode_info *OpCode)
{
    u32 Base = ASM_SELECT0(OpCode->Select0) | ASM_RS(OpCode->FunctionSelect) | ASM_RT(OpCode->RightValue);
    if (OpCode->FunctionSelect == 0b01000) Base |= ASM_IMM16(OpCode->Immediate);
    else Base |= ASM_RD(OpCode->DestinationRegister);
    return Base;
}

static u32
TranslateSecondary(disasm_opcode_info *OpCode)
{
    return SecondaryJumpTable[OpCode->Select1](OpCode);
}

void
AssemblerTranslateOpCode(disasm_opcode_info *OpCode, u32 *Data)
{
    *Data = PrimaryJumpTable[OpCode->Select0](OpCode);
}

void
AssemblerTranslateOpCodeArray(disasm_opcode_info *OpCodes, u32 n, u32 *Data)
{
    for (u32 i = 0; i < n; ++i)
    {
        AssemblerTranslateOpCode(&OpCodes[i], &Data[i]);
    }
}

static void __attribute__((constructor))
InitJumpTables()
{
    for (int i = 0; i <= 0x3F; ++i)
    {
        PrimaryJumpTable[i] = UnknownOpCode;
        SecondaryJumpTable[i] = UnknownOpCode;
    }

    PrimaryJumpTable[0x00] = TranslateSecondary;
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
    PrimaryJumpTable[0x10] = Coprocessor;
    PrimaryJumpTable[0x11] = Coprocessor;
    PrimaryJumpTable[0x12] = Coprocessor;
    PrimaryJumpTable[0x13] = Coprocessor;
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

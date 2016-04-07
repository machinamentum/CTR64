/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef DISASM_H
#define DISASM_H

#include "platform.h"
#include "mips.h"

struct disasm_opcode_info
{
    u32 Select0;
    u32 Select1;
    u32 LeftValue;
    u32 RightValue;
    u64 Immediate;
    u32 Result;
    u32 FunctionSelect;
    u32 MemAccessType;
    u32 MemAccessMode;
    u32 DestinationRegister;
    u32 RADestinationRegister; // Used for return address writing
    u64 CurrentAddress;
};

void DisassemblerPrintRange(MIPS_R3000 *Cpu, u64 Base, u32 Count, u64 PC);
void DisassemblerDecodeOpcode(disasm_opcode_info *OpCode, u32 Data, u64 IAddress);
void DisassemblerPrintOpCode(disasm_opcode_info *OpCode);
void DisassemblerSetPrintFunction(int (*)(const char *, ...));
const char *DisassemblerGetGPRName(u32 Reg);
void DumpState(MIPS_R3000 *Cpu);
void DumpC0State(Coprocessor *C0);

#endif

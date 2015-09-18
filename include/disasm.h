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

void DisassemblerPrintRange(MIPS_R3000 *Cpu, u32 Base, u32 Count, u32 PC);
void DumpState(MIPS_R3000 *Cpu);
void DumpC0State(Coprocessor *C0);

#endif

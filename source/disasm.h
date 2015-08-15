
#ifndef DISASM_H
#define DISASM_H

#include <3ds/types.h>
#include "mips.h"

void DisassemblerPrintRange(MIPS_R3000 *Cpu, u32 Base, u32 Count, u32 PC);


#endif

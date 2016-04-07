/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef ASM_H
#define ASM_H

#include "disasm.h"

#define ASM_SELECT0(x)    (((u32)x & 0b111111) << 26)
#define ASM_SELECT1(x)    (((u32)x & 0b111111) <<  0)
#define ASM_RS(x)         (((u32)x & 0b011111) << 21)
#define ASM_RT(x)         (((u32)x & 0b011111) << 16)
#define ASM_RD(x)         (((u32)x & 0b011111) << 11)
#define ASM_COMMENT20(x)  (((u32)x & 0x0FFFFF) <<  6)
#define ASM_CODE10(x)     (((u32)x & 0x0003FF) <<  6)
#define ASM_IMM5(x)       (((u32)x & 0b011111) <<  6)
#define ASM_IMM16(x)      (((u32)x & 0x00FFFF) <<  0)
#define ASM_IMM25(x)      (((u32)x & 0x1FFFFFF) << 0)
#define ASM_IMM26(x)      (((u32)x & 0x3FFFFFF) << 0)

#define ASM_UNK_OP  __builtin_bswap32(0x00BABE00)

void AssemblerTranslateOpCode(disasm_opcode_info *OpCode, u32 *Data);
void AssemblerTranslateOpCodeArray(disasm_opcode_info *OpCodes, u32 n, u32 *Data);

#endif

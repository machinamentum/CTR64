#include <asm/regdef.h>
    .text
    .global _start
_start:
    nop
    nop
    sw $0, 0x80
    nop
    nop
    b _start
    .rdata
    .asciz "CTRX BIOS"

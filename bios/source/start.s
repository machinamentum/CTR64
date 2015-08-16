#include <asm/regdef.h>
    .set		noreorder
    .section .text.boot
    .global _start
_start:
    nop
    nop
    nop
    # TODO start up code
    b _kernel_proper
    nop
    .section .rodata.rom_header
    .int KERNEL_BUILD_DATE
    .int 0x43545258
    .asciz "CTRX BIOS"

    .section .text
_kernel_proper:
    b _start

#include <asm/regdef.h>
    .section .text.boot
    .global _start
_start:
    # TODO start up code
    b _kernel_proper

    .section .rodata.rom_header
    .int KERNEL_BUILD_DATE
    .int 0x43545258
    .asciz "CTRX BIOS"

    .section .text
_kernel_proper:
    b _start

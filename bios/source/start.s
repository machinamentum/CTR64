/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 94):
* <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
* ----------------------------------------------------------------------------
*/
    .include "regdef.h"
    .include "asm_macros.h"
    .set		noreorder
    .section .text.boot
    .global _start
_start:
    nop
    mtc0 zr, sr
    nop
    lui sp, 0x801F
    ori sp, sp, 0xFF00
    nop
    j kmain
    nop

    .section .rodata.rom_header
    .global _kernel_build_date
    .global _kernel_flags
    .global _kernel_ascii_id
_kernel_build_date:
    .int KERNEL_BUILD_DATE
_kernel_flags:
    .int 0x43545258
_kernel_ascii_id:
    .asciz "CTR64 IPL"

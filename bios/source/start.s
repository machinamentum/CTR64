/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 94):
* <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
* ----------------------------------------------------------------------------
*/
    .include "regdef.h"
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
    .int KERNEL_BUILD_DATE
    .int 0x43545258
    .asciz "CTRX BIOS"

    .text
    .global _jump_redirect_A
_jump_redirect_A:
    j _jump_func_A

    .global _jump_redirect_B
_jump_redirect_B:
    j _jump_func_B


_jump_func_A:
    ori a0, zr, 8
    mul a0, $9, a0
    la a1, _jump_table_A
    addu a1, a1, a0
    jr a1

_jump_func_B:
    ori a0, zr, 8
    mul a0, $9, a0
    la a1, _jump_table_B
    addu a1, a1, a0
    jr a1


_jump_table_A:
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j memcpy
    nop

_jump_table_B:
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SetDefaultExitFromException
    nop
    j SetCustomExitFromException
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop

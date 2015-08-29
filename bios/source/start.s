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
    .asciz "CTRX BIOS"

    .text
    .global _exception_handler_entry
_exception_handler_entry:
    j _exception_handler_entry_main
    nop
_exception_handler_size:
    .word . - _exception_handler_entry

_exception_handler_entry_main:
    pushall
    mfc0 a0, cause
    mfc0 a1, epc
    jal KernelHandleException
    nop
    popall
    mfc0 k0, epc
    addiu k0, k0, 4
    jr k0
    rfe
    nop
    .global _exception_handler_size

    .global _jump_redirect_A
_jump_redirect_A:
    j _jump_func_A
    nop

    .global _jump_redirect_B
_jump_redirect_B:
    j _jump_func_B
    nop


_jump_func_A:
    ori t7, zr, 8
    mul t7, $9, t7
    la t8, _jump_table_A
    addu t8, t8, t7
    jr t8
    nop

_jump_func_B:
    ori t7, zr, 8
    mul t7, $9, t7
    la t8, _jump_table_B
    addu t8, t8, t7
    jr t8
    nop


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
    j todigit
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j abs
    nop
    j labs
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
    j strlen
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

    .global _jump_table_B
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
    j ReturnFromException
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

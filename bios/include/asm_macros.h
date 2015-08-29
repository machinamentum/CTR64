/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
    .include "regdef.h"

    .macro push rs
    addiu sp, sp, -4
    sw \rs, 0(sp)
    .endm

    .macro pop rs
    lw \rs, 0(sp)
    addiu sp, sp, 4
    .endm

    .macro pushall
    .set push
    .set noat
    push at
    push v0
    push v1
    push a0
    push a1
    push a2
    push a3
    push t0
    push t1
    push t2
    push t3
    push t4
    push t5
    push t6
    push t7
    push s0
    push s1
    push s2
    push s3
    push s4
    push s5
    push s6
    push s7
    push t8
    push t9
    push k0
    push k1
    push gp
    push fp
    push ra
    .set pop
    .endm

    .macro popall
    .set push
    .set noat
    pop ra
    pop fp
    pop gp
    pop k1
    pop k0
    pop t9
    pop t8
    pop s7
    pop s6
    pop s5
    pop s4
    pop s3
    pop s2
    pop s1
    pop s0
    pop t7
    pop t6
    pop t5
    pop t4
    pop t3
    pop t2
    pop t1
    pop t0
    pop a3
    pop a2
    pop a1
    pop a0
    pop v1
    pop v0
    pop at
    .set pop
    .endm

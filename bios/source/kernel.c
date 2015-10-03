/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

const int *RAMROM_BOOTSTRAP_OFFSET = (int *)0x10000040;

void kmain(void)
{
    void *RSP_DMEM = (int *)0xA4000040;
    memcpy(RSP_DMEM, RAMROM_BOOTSTRAP_OFFSET, 1008 * 4);
    void (*UserCode)() = RSP_DMEM;
    UserCode();
    while (1){}
}

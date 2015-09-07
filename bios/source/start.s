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
    .global _enable_interrupts
_enable_interrupts:
    addiu t0, zr, 1
    mtc0 t0, sr
    jr ra
    nop

    .global _exception_handler_entry
_exception_handler_entry:
    j _exception_handler_entry_main
    nop
_exception_handler_size:
    .word . - _exception_handler_entry

_exception_handler_entry_main:
    pushall
    addiu a3, a1, 0
    addiu a2, a0, 0
    mfc0 a0, cause
    mfc0 a1, epc
    jal ExceptionHandler
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

    .global _jump_redirect_C
_jump_redirect_C:
    j _jump_func_C
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

_jump_func_C:
    ori t7, zr, 8
    mul t7, $9, t7
    la t8, _jump_table_C
    addu t8, t8, t7
    jr t8
    nop

_jump_table_A:
    j FileOpen
    nop
    j FileSeek
    nop
    j FileRead
    nop
    j FileWrite
    nop
    j FileClose
    nop
    j FileIoctl
    nop
    j exit
    nop
    j FileGetDeviceFlag
    nop
    j FileGetc
    nop
    j FilePutc
    nop
    j todigit
    nop
    j atof
    nop
    j strtoul
    nop
    j strtol
    nop
    j abs
    nop
    j labs
    nop
    j atoi
    nop
    j atol
    nop
    j atob
    nop
    j SaveState
    nop
    j RestoreState
    nop
    j strcat
    nop
    j strncat
    nop
    j strcmp
    nop
    j strncmp
    nop
    j strcpy
    nop
    j strncpy
    nop
    j strlen
    nop
    j index
    nop
    j rindex
    nop
    j strchr
    nop
    j strrchr
    nop
    j strpbrk
    nop
    j strspn
    nop
    j strcspn
    nop
    j strtok
    nop
    j strstr
    nop
    j toupper
    nop
    j tolower
    nop
    j bcopy
    nop
    j bzero
    nop
    j bcmp
    nop
    j memcpy
    nop
    j memset
    nop
    j memmove
    nop
    j memcmp
    nop
    j memchr
    nop
    j rand
    nop
    j srand
    nop
    j qsort
    nop
    j strtod
    nop
    j malloc
    nop
    j free
    nop
    j lsearch
    nop
    j bsearch
    nop
    j calloc
    nop
    j realloc
    nop
    j InitHeap
    nop
    j SystemErrorExit
    nop
    j std_in_getchar
    nop
    j std_out_putchar
    nop
    j std_in_gets
    nop
    j std_out_puts
    nop
    j printf
    nop
    j SystemErrorUnresolvedException
    nop
    j LoadExeHeader
    nop
    j LoadExeFile
    nop
    j DoExecute
    nop
    j FlushCache
    nop
    j init_a0_b0_c0_vectors
    nop
    j GPU_dw
    nop
    j gpu_send_dma
    nop
    j SendGP1Command
    nop
    j GPU_cw
    nop
    j GPU_cwp
    nop
    j send_gpu_linked_list
    nop
    j gpu_abort_dma
    nop
    j GetGPUStatus
    nop
    j gpu_sync
    nop
    j SystemError
    nop
    j SystemError
    nop
    j LoadAndExecute
    nop
    j SystemError
    nop
    j SystemError
    nop
    j CdInit
    nop
    j _bu_init
    nop
    j CdRemove
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j dev_tty_init
    nop
    j dev_tty_open
    nop
    j dev_tty_in_out
    nop
    j dev_tty_ioctl
    nop
    j dev_cd_open
    nop
    j dev_cd_read
    nop
    j dev_cd_close
    nop
    j dev_cd_firstfile
    nop
    j dev_cd_nextfile
    nop
    j dev_cd_chdir
    nop
    j dev_card_open
    nop
    j dev_card_read
    nop
    j dev_card_write
    nop
    j dev_card_close
    nop
    j dev_card_firstfile
    nop
    j dev_card_nextfile
    nop
    j dev_card_erase
    nop
    j dev_card_undelete
    nop
    j dev_card_format
    nop
    j dev_card_rename
    nop
    j SystemError /* card_clear_error ? */
    nop
    j _bu_init
    nop
    j CdInit
    nop
    j CdRemove
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j CdAsyncSeekL
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j CdAsyncGetStatus
    nop
    j EmptyReturn
    nop
    j CdAsyncReadSector
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j CdAsyncSetMode
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j CdromIoIrqFunc1
    nop
    j CdromDmaIrqFunc1
    nop
    j CdromIoIrqFunc2
    nop
    j CdromDmaIrqFunc2
    nop
    j CdromGetInt5errCode
    nop
    j CdInitSubFunc
    nop
    j AddCDROMDevice
    nop
    j AddMemCardDevice
    nop
    j AddDuartTtyDevice
    nop
    j AddDummyTtyDevice
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SetConf
    nop
    j GetConf
    nop
    j SetCdromIrqAutoAbort
    nop
    j SetMemSize
    nop
    j WarmBoot
    nop
    j SystemErrorBootOrDiskFailure
    nop
    j EnqueueCdIntr
    nop
    j DequeueCdIntr
    nop
    j CdGetLbn
    nop
    j CdReadSector
    nop
    j CdGetStatus
    nop
    j bu_callback_okay
    nop
    j bu_callback_err_write
    nop
    j bu_callback_err_busy
    nop
    j bu_callback_err_eject
    nop
    j _card_info
    nop
    j _card_async_load_directory
    nop
    j set_card_auto_format
    nop
    j bu_callback_err_prev_write
    nop
    j card_write_test
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j ioabort_raw
    nop
    j EmptyReturn
    nop
    j GetSystemInfo
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    .global _jump_table_B
_jump_table_B:
    j alloc_kernel_memory
    nop
    j free_kernel_memory
    nop
    j init_timer
    nop
    j get_timer
    nop
    j enable_timer_irq
    nop
    j disable_timer_irq
    nop
    j restart_timer
    nop
    j DeliverEvent
    nop
    j OpenEvent
    nop
    j CloseEvent
    nop
    j WaitEvent
    nop
    j TestEvent
    nop
    j EnableEvent
    nop
    j DisableEvent
    nop
    j OpenThread
    nop
    j CloseThread
    nop
    j ChangeThread
    nop
    j 0
    nop
    j InitPad
    nop
    j StartPad
    nop
    j StopPad
    nop
    j OutdatedPadInitAndStart
    nop
    j OutdatedPadGetButtons
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
    j UnDeliverEvent
    nop
    j SystemError
    nop
    j SystemError
    nop
    j SystemError
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j SystemError
    nop
    j SystemError
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j FileOpen
    nop
    j FileSeek
    nop
    j FileRead
    nop
    j FileWrite
    nop
    j FileClose
    nop
    j FileIoctl
    nop
    j exit
    nop
    j FileGetDeviceFlag
    nop
    j FileGetc
    nop
    j FilePutc
    nop
    j std_in_getchar
    nop
    j std_out_putchar
    nop
    j std_in_gets
    nop
    j std_out_puts
    nop
    j chdir
    nop
    j FormatDevice
    nop
    j firstfile
    nop
    j nextfile
    nop
    j FileRename
    nop
    j FileDelete
    nop
    j FileUndelete
    nop
    j AddDevice
    nop
    j RemoveDevice
    nop
    j PrintInstalledDevices
    nop
    j InitCard
    nop
    j StartCard
    nop
    j StopCard
    nop
    j _card_info_subfunc
    nop
    j write_card_sector
    nop
    j read_card_sector
    nop
    j allow_new_card
    nop
    j Krom2RawAdd
    nop
    j SystemError
    nop
    j Krom2Offset
    nop
    j GetLastError
    nop
    j GetLastFileError
    nop
    j GetC0Table
    nop
    j GetB0Table
    nop
    j get_bu_callback_port
    nop
    j testdevice
    nop
    j SystemError
    nop
    j ChangeClearPad
    nop
    j get_card_status
    nop
    j wait_card_status
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop

    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop
    j 0
    nop


    .global _jump_table_C
_jump_table_C:
    j EnqueueTimerAndVblankIrqs
    nop
    j EnqueueSyscallHandler
    nop
    j SysEnqIntRP
    nop
    j SysDeqIntRP
    nop
    j get_free_EvCB_slot
    nop
    j get_free_TCB_slot
    nop
    j ExceptionHandler
    nop
    j InstallExceptionHandlers
    nop
    j SysInitMemory
    nop
    j SysInitKernelVariables
    nop
    j ChangeClearRCnt
    nop
    j SystemError
    nop
    j InitDefInt
    nop
    j SetIrqAutoAck
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j EmptyReturn
    nop
    j InstallDevices
    nop
    j FlushStdInOutPut
    nop
    j EmptyReturn
    nop
    j tty_cdevinput
    nop
    j tty_cdevscan
    nop
    j tty_circgetc
    nop
    j tty_circputc
    nop
    j ioabort
    nop
    j set_card_find_mode
    nop
    j KernelRedirect
    nop
    j AdjustA0Table
    nop
    j get_card_find_mode
    nop
    j 0
    nop



/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

static inline void
EnableDisplay()
{
    // Reset GPU
    SendGP1Command(0);

    // turn on display
    SendGP1Command(GPCOM(0x03, 0));

    // fill rect black
    GPU_cw(GPCOM(0x02, 0x000000));
    GPU_cw(0); // xy
    GPU_cw((240 << 16) | (320));//width, height

    //enable drawing to display
    GPU_cw(GPCOM(0xE1, 1 << 10));

    //set drawing area
    GPU_cw(GPCOM(0xE3, 0));
    GPU_cw(GPCOM(0xE4, 240 | (320 << 10)));
}

const static unsigned short Quad[8] = {
    320, 240,
    0, 240,
    320, 0,
    0, 0
};

static void
DrawQuad(const unsigned short *Data)
{
    GPU_cw(GPCOM(0x28, 0xDB7093));
    GPU_cw(Data[0] | (Data[1] << 16));
    GPU_cw(Data[2] | (Data[3] << 16));
    GPU_cw(Data[4] | (Data[5] << 16));
    GPU_cw(Data[6] | (Data[7] << 16));
}

int
init_timer(int Timer, int Reload, int Flags)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
get_timer(int Timer)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
enable_timer_irq(int Timer)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
disable_timer_irq(int Timer)
{
    printf("%s\n", __FUNCTION__);
    return 1;
}

int
restart_timer(int Timer)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
ChangeClearRCnt(int Timer, int Flag)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
Krom2Offset(int JIS)
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

int
Krom2RawAdd(int JIS)
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

void
InitDefInt(int Priority)
{
    printf("%s\n", __FUNCTION__);
}

void
EnqueueSyscallHandler(int Priority)
{
    printf("%s\n", __FUNCTION__);
}

void
EnqueueTimerAndVblankIrqs(int Priority)
{
    printf("%s\n", __FUNCTION__);
}

void
SysEnqIntRP(int Priority, void *Struc)
{
    printf("%s\n", __FUNCTION__);
}

void
SysDeqIntRP(int Priority, void *Struc)
{
    printf("%s\n", __FUNCTION__);
}

void
SetIrqAutoAck(int IRQ, int Flag)
{
    printf("%s\n", __FUNCTION__);
}

void
ReturnFromException()
{
    printf("%s\n", __FUNCTION__);
    // TODO
}

static FunctionHook ExitFromExceptionHook;
static const FunctionHook DefaultExitFromExceptionHook =
{
    &ReturnFromException,
    0x800001F0 - 4, //temporary stacktop
    0,
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    0
};

FunctionHook *
SetDefaultExitFromException()
{
    printf("%s\n", __FUNCTION__);
    memcpy(&ExitFromExceptionHook, &DefaultExitFromExceptionHook, sizeof(FunctionHook));
    return &ExitFromExceptionHook;
}

void
SetCustomExitFromException(FunctionHook *Hook)
{
    printf("%s\n", __FUNCTION__);
    memcpy(&ExitFromExceptionHook, Hook, sizeof(FunctionHook));
}

int
std_in_getchar()
{
    printf("%s\n", __FUNCTION__);
    char Temp;
    FileRead(0, &Temp, 1);
    return Temp & 0x7F;
}

void *
std_in_gets(void *Dst)
{
    printf("%s\n", __FUNCTION__);
    // TODO
    return Dst;
}

void
std_out_puts(const char *Src)
{
    if (!Src)
    {
        std_out_puts("<NULL>");
        return;
    }
    while (*Src != 0)
    {
        std_out_putchar(*Src);
        ++Src;
    }
}

void
std_out_putchar(const char Src)
{
    if (!Src)
    {
        std_out_puts("<NULL>");
        return;
    }

    unsigned int *const CTRX_PRINT_STR = (unsigned int *)0x1F802064;
    *CTRX_PRINT_STR = (unsigned int)Src;
}

int
SystemError()
{
    printf("%s\n", __FUNCTION__);
    return 0;
}


unsigned int
GetSystemInfo(unsigned int Index)
{
    printf("%s\n", __FUNCTION__);
    extern const unsigned int _kernel_build_date;
    extern const unsigned int _kernel_flags;
    extern const unsigned int _kernel_ascii_id;
    switch (Index)
    {
        case 0:
            return _kernel_build_date;
        case 1:
            return _kernel_flags;
        case 2:
            return (unsigned int)&_kernel_ascii_id;
        case 3:
        case 4:
            return 0;
        case 5:
            return 2048;
        default:
            return 0;
    }
}


void *
GetB0Table()
{
    printf("%s\n", __FUNCTION__);
    extern const unsigned int _jump_table_B;
    return (void *)&_jump_table_B;
}

void *
GetC0Table()
{
    printf("%s\n", __FUNCTION__);
    extern const unsigned int _jump_table_C;
    return (void *)&_jump_table_C;
}

void
AdjustA0Table()
{
    printf("%s\n", __FUNCTION__);
}

int
EnterCriticalSection()
{
    printf("%s\n", __FUNCTION__);
    int Result = 0;
    int SR;
    __asm__("mfc0 %0, $12" : "=r" (SR) ::);
    if ((SR & (1 << 2)) && (SR & (1 << 10)))
    {
        Result = 1;
    }
    SR &= ~((1 << 2) | (1 << 10));
    __asm__("mtc0 %0, $12" : "=r" (SR) ::);
    return Result;
}

void
ExitCriticalSection()
{
    printf("%s\n", __FUNCTION__);
    int SR;
    __asm__("mfc0 %0, $12" : "=r" (SR) ::);
    SR |= ((1 << 2) | (1 << 10));
    __asm__("mtc0 %0, $12" : "=r" (SR) ::);
}

void *
get_free_TCB_slot()
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

void
ChangeThreadSubFunction(void *Addr)
{
    printf("%s\n", __FUNCTION__);
}

void *
OpenThread(int reg_PC, int reg_SP_FP, int reg_GP)
{
    printf("%s\n", __FUNCTION__);
    return (void *)0xFFFFFFFF;
}

void *
CloseThread(void *Handle)
{
    printf("%s\n", __FUNCTION__);
    return (void *)1;
}

void *
ChangeThread(void *Handle)
{
    printf("%s\n", __FUNCTION__);
    return (void *)1;
}


void
ExceptionHandler(unsigned int Cause, unsigned int EPC, unsigned int Selector, void *Addr)
{
    Cause = (Cause >> 2) & 0x3F;
    if (Cause == 0x0A)
    {
        std_out_puts("Reserved Instruction exception\n");
    }

    if (Cause == 0x8)
    {
        std_out_puts("Syscall\n");
        switch (Selector)
        {
            case 0x01:
                EnterCriticalSection();
                break;
            case 0x02:
                ExitCriticalSection();
                break;
            case 0x03:
                ChangeThreadSubFunction(Addr);
                break;
            default:
                DeliverEvent(0xF0000010, 0x4000);
                break;
        }

    }
    if (Cause == 0x9)
    {
        std_out_puts("BRKPT\n");
    }
    if (Cause == 0)
    {
        std_out_puts("Interrupt\n");
    }

    ReturnFromException();
}

void
SetConf(int NumEvCB, int NumTCB, void *StackTop)
{
    printf("%s\n", __FUNCTION__);
}

void
GetConf(int *NumEvCB, int *NumTCB, void **StackTop)
{
    printf("%s\n", __FUNCTION__);
}

void
FlushCache()
{
    printf("%s\n", __FUNCTION__);
}

void
FlushStdInOutPut()
{
    printf("%s\n", __FUNCTION__);
    FileClose(0);
    FileClose(1);
//    FileOpen(0, FILE_READ);
//    FileOpen(1, FILE_WRITE);
}

void
InstallExceptionHandlers()
{
    printf("%s\n", __FUNCTION__);
    unsigned int *GeneralVector = (unsigned int *)0x80000080;
    extern const unsigned int _exception_handler_size;
    extern unsigned int _exception_handler_entry;
    unsigned int *ExceptionHandler = &_exception_handler_entry;
    for (int i = 0; i < _exception_handler_size / 4; ++i)
    {
        GeneralVector[i] = ExceptionHandler[i];
    }
}

void
SystemErrorExit(int ExitCode)
{
    printf("%s\n", __FUNCTION__);
    while (1);
}

void
SystemErrorUnresolvedException()
{
    printf("%s\n", __FUNCTION__);
    while (1);
}

void
SystemErrorBootOrDiskFailure(int Type, int ErrorCode)
{
    printf("%s\n", __FUNCTION__);
    while (1);
}


int
EmptyReturn()
{
    return 0;
}

void
WarmBoot()
{
    printf("%s\n", __FUNCTION__);
}

int
SaveState(void *Buf)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

void
RestoreState(void *Buf, int Param)
{
    printf("%s\n", __FUNCTION__);
}

char ioabortbuffer[0x40];

void
ioabort_raw(int Param)
{
    printf("%s\n", __FUNCTION__);
    RestoreState(ioabortbuffer, Param);
}

void
ioabort(const char *Txt1, const char *Txt2)
{
    printf("%s\n", __FUNCTION__);
    std_out_puts(Txt1);
    std_out_puts(Txt2);
    ioabort_raw(1);
}

void
KernelRedirect(int TtyFlag)
{
    printf("%s\n", __FUNCTION__);
}

void
SetMemSize(int Size)
{
    printf("%s\n", __FUNCTION__);
}

void *KernelMemStart;
void *KernelMemPtr;
int KernelMemSize;

void *
alloc_kernel_memory(int Size)
{
    printf("%s\n", __FUNCTION__);
    if (Size % 4) Size += (4 - (Size % 4));
    if (KernelMemStart + KernelMemSize < KernelMemPtr + Size) return 0;
    void *Temp = KernelMemPtr;
    KernelMemPtr += Size;
    return Temp;
}

void
free_kernel_memory(void *Ptr)
{
    printf("%s\n", __FUNCTION__);
    unsigned int *Buf = (unsigned int *)(((char *)Ptr) - 4);
    Buf[0] |= 1;
}

void
SysInitMemory(void *Address, int Size)
{
    printf("%s\n", __FUNCTION__);
    KernelMemStart = KernelMemPtr = Address;
    KernelMemSize = Size;
}

void
SysInitKernelVariables()
{
    printf("%s\n", __FUNCTION__);
}

static void
InstallBIOSJumperCables()
{
    extern const unsigned int _jump_redirect_A;
    extern const unsigned int _jump_redirect_B;
    extern const unsigned int _jump_redirect_C;
    unsigned int *const ACable = (unsigned int *)0x000000A0;
    unsigned int *const BCable = (unsigned int *)0x000000B0;
    unsigned int *const CCable = (unsigned int *)0x000000C0;
    *ACable = _jump_redirect_A;
    *BCable = _jump_redirect_B;
    *CCable = _jump_redirect_C;
}

void
init_a0_b0_c0_vectors()
{
    printf("%s\n", __FUNCTION__);
    InstallBIOSJumperCables();
}

void kmain(void)
{
    std_out_puts("CTRX BIOS by machinamentum\n");
    InstallExceptionHandlers();
    EnableDisplay();
    DrawQuad(Quad);
    std_out_puts("Attaching jumper cables...");
    InstallBIOSJumperCables();
    std_out_puts("done\n");
    InitHeap((void *)0x80000100, 0x10000 - 0x100);

    std_out_puts("Loading system.cnf from CDROM\n");
    CdInit();
    int SysCnfFile = FileOpen("cdrom:SYSTEM.CNF", FILE_READ);
    char *Buffer = malloc(512);
    int ReadBytes = FileRead(SysCnfFile, Buffer, 512);
    printf("Read %X bytes\n", ReadBytes);
    printf("%s\n", Buffer);

    char *ExeNameStart = Buffer;
    while (*ExeNameStart != 'c')
    {
        ++ExeNameStart;
    }

    char *ExeNameEnd = ExeNameStart;
    while (*ExeNameEnd != ';')
    {
        ++ExeNameEnd;
    }
    *ExeNameEnd = 0;
    std_out_puts("Enabling exceptions\n");
    extern void _enable_interrupts();
    _enable_interrupts();
    printf("Loading and running PSX EXE: %s\n", ExeNameStart);
    LoadAndExecute(ExeNameStart, 0, 0);
    while (1){}
}

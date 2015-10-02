/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef KERNEL_H
#define KERNEL_H

typedef struct
{
    void (*FuncPtr)(void);
    unsigned int StackPtr;
    unsigned int FramePtr;
    unsigned int SavedVars[8];
    unsigned int GlobalPtr;
} FunctionHook;

void *memcpy(void *Dst, const void *Src, unsigned int Length);

void *memmove(void *Dst, const void *Src, unsigned int Length);

void *memset(const void *Dst, int FillByte, unsigned int Length);

int memcmp(const void *Src1, const void *Src2, unsigned int Length);

void *memchr(const void *Src, char ScanByte, unsigned int Length);

void *malloc(int Size);

void free(void *Ptr);

void InitHeap(void *Address, int Size);

void ReturnFromException();

FunctionHook *SetDefaultExitFromException();

void SetCustomExitFromException(FunctionHook *Hook);

void std_out_puts(const char *Src);

void std_out_putchar(const char Src);

void printf(const char *fmt, ...);

int SystemError();

unsigned int GetSystemInfo(unsigned int Index);

unsigned int rand();
void srand(unsigned int Seed);

int abs(int Value);
long int labs(long int Value);
unsigned int todigit(unsigned int c);

unsigned int strlen(const char *Src);

void *GetB0Table();

void KernelRedirect(int TtyFlag);
int EnterCriticalSection();
void ExitCriticalSection();
void ExceptionHandler(unsigned int Cause, unsigned int EPC, unsigned int Selector, void *Addr);
void InstallExceptionHandler();

#define EVENT_VBLANK     (0xF0000001)
#define EVENT_GPU        (0xF0000002)
#define EVENT_CDROM      (0xF0000003)
#define EVENT_DMA        (0xF0000004)
#define EVENT_RTC0       (0xF0000005)
#define EVENT_RTC1       (0xF0000006)
#define EVENT_JOYPAD     (0xF0000008)
#define EVENT_SPU        (0xF0000009)
#define EVENT_PIO        (0xF000000A)
#define EVENT_SIO        (0xF000000B)
#define EVENT_CRASH      (0xF0000010)
#define EVENT_MEMCARD    (0xF0000011)

#define EVENT_DOTCLOCK   (0xF2000000)
#define EVENT_HRETRACE   (0xF2000001)
#define EVENT_SLOWCLOCK  (0xF2000002)
#define EVENT_VRETRACE   (0xF2000003)

void DeliverEvent(int Class, int Spec);

#define FILE_READ         (1 << 0)
#define FILE_WRITE        (1 << 1);
#define FILE_NON_BLOCKING (1 << 2);
#define FILE_CREATE       (1 << 9);
#define FILE_ASYNC        (1 << 15);
#define FILE_CARD_BLOCKS(x) (x << 16)

#define FILE_SEEK_START    (0)
#define FILE_SEEK_CURRENT  (1)
#define FILE_SEEK_END      (2)

int FileOpen(const char *FileName, int AccessMode);

int FileSeek(int Fd, int Offset, int SeekType);

int FileRead(int Fd, void *Dst, int Length);

int FileWrite(int Fd, void *Src, int Length);

int FileClose(int Fd);

int FileGetc(int Fd);

int FilePutc(char Char, int Fd);

int chdir(const char *Name);

void SystemErrorExit(int ExitCode);

void SystemErrorUnresolvedException();

void SystemErrorBootOrDiskFailure(int Type, int ErrorCode);

void exit(int ExitCode);

int FileIoctl(int Fd, int Cmd, ...);

float atof(const char *Str);

#define GPCOM(func, param) \
(((func & 0xFF) << 24) | ((param) & 0x00FFFFFF))

void SendGP1Command(unsigned int Cmd);
int GPU_cw(unsigned int Cmd);
unsigned int GetGPUStatus();
int gpu_sync();

void CdInit();

typedef struct
{
    char FileName[0x14];
    int Attribute;
    int Size;
    void *Next;
    int SelectorNumber;
    int Reserved;
} DirEntry;

void LoadAndExecute(const char *FileName, unsigned int StackBase, unsigned int StackOffset);


#endif

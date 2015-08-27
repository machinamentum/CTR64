/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>

#define GPCOM(func, param) \
(((func & 0xFF) << 24) | ((param) & 0x00FFFFFF))

uint32_t * const GP0 = (uint32_t *)0x1F801810;
uint32_t * const GP1 = (uint32_t *)0x1F801814;
//static const uint32_t *GPUREAD = (uint32_t *)0x1F801810;
//static const uint32_t *GPUSTAT = (uint32_t *)0x1F801814;

static inline void
WriteGP0(uint32_t command)
{
    *GP0 = command;
}

static inline void
WriteGP1(uint32_t command)
{
    *GP1 = command;
}

static inline void
EnableDisplay()
{
    // Reset GPU
    WriteGP1(0);

    // turn on display
    WriteGP1(GPCOM(0x03, 0));

    // fill rect black
    WriteGP0(GPCOM(0x02, 0x000000));
    WriteGP0(0); // xy
    WriteGP0((240 << 16) | (320));//width, height

    //enable drawing to display
    WriteGP0(GPCOM(0xE1, 1 << 10));

    //set drawing area
    WriteGP0(GPCOM(0xE3, 0));
    WriteGP0(GPCOM(0xE4, 240 | (320 << 10)));
}

const static uint16_t Quad[8] = {
    320, 240,
    0, 240,
    320, 0,
    0, 0
};

static void
DrawQuad(const uint16_t *Data)
{
    WriteGP0(GPCOM(0x28, 0xDB7093));
    WriteGP0(Data[0] | (Data[1] << 16));
    WriteGP0(Data[2] | (Data[3] << 16));
    WriteGP0(Data[4] | (Data[5] << 16));
    WriteGP0(Data[6] | (Data[7] << 16));
}

typedef struct
{
    void (*FuncPtr)(void);
    uint32_t StackPtr;
    uint32_t FramePtr;
    uint32_t SavedVars[8];
    uint32_t GlobalPtr;
} FunctionHook;

void
memcpy(const void *Dst, const void *Src, uint32_t Len)
{
    if (!Dst) return;
    if (Len > 0x7FFFFFFF) return;
    char *DstB = (char *)Dst;
    char *SrcB = (char *)Src;
    for (int i = 0; i < Len; ++i)
    {
        DstB[i] = SrcB[i];
    }
}

static void
ReturnFromException()
{
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

FunctionHook *SetDefaultExitFromException()
{
    memcpy(&ExitFromExceptionHook, &DefaultExitFromExceptionHook, sizeof(FunctionHook));
    return &ExitFromExceptionHook;
}

void SetCustomExitFromException(FunctionHook *Hook)
{
    memcpy(&ExitFromExceptionHook, Hook, sizeof(FunctionHook));
}

int SystemError()
{
    return 0;
}

static void InstallBIOSJumperCables()
{
    extern const uint32_t _jump_redirect_A;
    extern const uint32_t _jump_redirect_B;
    uint32_t *const ACable = (uint32_t *)0x000000A0;
    uint32_t *const BCable = (uint32_t *)0x000000B0;
//    uint32_t *const CCable = (uint32_t *)0x000000C0;
    *ACable = _jump_redirect_A;
    *BCable = _jump_redirect_B;
}

void kmain(void)
{
    EnableDisplay();
    DrawQuad(Quad);
    InstallBIOSJumperCables();
    typedef void (*UEFunc)(void);
    UEFunc UserEntry = (UEFunc)0x80010000;
    UserEntry();
    while (1){}
}

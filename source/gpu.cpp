/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "gpu.h"
#include <gfx_device.h>
#include <GL/gl.h>

#include <cstdio>

static void
GteExecuteOperation(Coprocessor *Cp, u32 FunctionCode)
{

}

GPU::
GPU()
{
    ExecuteOperation = GteExecuteOperation;

    GfxHandle = gfxCreateDevice(240, 400);
    gfxMakeCurrent(GfxHandle);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 1.0, 0.0, -1.0, 1.0);

    glTranslatef(0.5f, 0.5f, 0.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);

    glScalef(1.0 / 320.0, 1.0/240.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);
}

typedef void (*gpu_func)(GPU *, u32);
static gpu_func GP0FuncTable[0xFF];

static void
GpuExecuteCommand(GPU *Gpu, u8 Command, u32 Param)
{
    GP0FuncTable[Command](Gpu, Param);
}

u32
GpuStat(void *Object, u32 Address)
{
    GPU *Gpu = (GPU *)Object;
    u32 Status = Gpu->Status;
    Status |= (1 << 26) | (1 << 27) | (1 << 28); // ready flags
    printf("GPUSTAT\n");
    return Status;
}

static u32 *PacketStore = (u32 *)linearAlloc(4 * 1000);
static u32 Packets = 0;

static void
Gp0Cpu2VRAM(GPU* Gpu, u32 Packet)
{
    u32 Width = (Gpu->Gp0Packets[0] & 0xFFFF) / 2;
    u32 Height = (Gpu->Gp0Packets[0] >> 16) & 0xFF;
    u32 Total = Width * Height;
    if (Packets < Total)
    {
        PacketStore[Packets++] = Packet;
        return;
    }
    u32 X = (Gpu->Gp0Packets[1] & 0xFFFF) / 2;
    u32 Y = (Gpu->Gp0Packets[1] >> 16) & 0xFFFF;
    u32 Base = (X + Y * 1024);
    u32 *VRAM = Gpu->VRAM;
    for (u32 j = 0; j < Width; j++)
    {
        for (u32 i = 0; i < Width; ++i)
        {
            VRAM[Base + i + j * 1024] = PacketStore[i + j * Width];
        }
    }
    Gpu->Gp0Func = NULL;
    printf("CPU2VRAM\n");
}

void
GpuGp0(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;

    if (Gpu->Gp0PacketsLeft)
    {
        Gpu->Gp0PacketsLeft--;
        Gpu->Gp0Packets[Gpu->Gp0PacketsLeft] = Value;
        if (Gpu->Gp0PacketsLeft == 0)
        {
            GpuExecuteCommand(Gpu, Gpu->Gp0WaitingCmd >> 24, Gpu->Gp0WaitingCmd & 0x00FFFFFF);
        }
        return;
    }

    if (Gpu->Gp0Func)
    {
        Gpu->Gp0Func(Gpu, Value);
        return;
    }
    
//    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    if (Command == GP0_COMMAND_FILL_RECT)
    {
        Gpu->Gp0WaitingCmd = Value;
        Gpu->Gp0PacketsLeft = 2;
    }

    if (Command == 0xA0)
    {
        Gpu->Gp0WaitingCmd = Value;
        Gpu->Gp0Func = Gp0Cpu2VRAM;
        Gpu->Gp0PacketsLeft = 2;
        Packets = 0;
    }

    if (Command == 0x28)
    {
        Gpu->Gp0WaitingCmd = Value;
        Gpu->Gp0PacketsLeft = 4;
    }

    printf("GP0 0x%08lX\n", Value);
}

void
GpuGp1(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    printf("GP1 0x%08lX\n", Value);

    switch (Command)
    {
        case GP1_COMMAND_RST:
            // TODO reset command
            break;
        case GP1_COMMAND_DISP_EN:
            Gpu->Status |= GPU_STAT_DISP_EN;
            break;
        case GP1_COMMAND_DMA_DIR:
            Gpu->Status ^= (-(Param & 3) ^ Gpu->Status) & (3 << 29);
            break;
    }


}

static void
GP0Nop(GPU *Gpu, u32 Param)
{

}

static void
GP0FillRect(GPU *Gpu, u32 Param)
{
    float R = (float)(Param & 0xFF) / 255.0f;
    float G = (float)((Param >> 8) & 0xFF) / 255.0f;
    float B = (float)((Param >> 16) & 0xFF) / 255.0f;
    u32 X = Gpu->Gp0Packets[1] & 0xFFFF;
    u32 Y = Gpu->Gp0Packets[1] >> 16;
    u32 W = Gpu->Gp0Packets[0] & 0xFFFF;
    u32 H = Gpu->Gp0Packets[0] >> 16;
    glColor3f(R, G, B);
    glBegin(GL_QUADS);
    glVertex2i(X, Y);
    glVertex2i(X + W, Y);
    glVertex2i(X + W, Y + H);
    glVertex2i(X, Y + H);
    glEnd();
    glColor4f(1, 1, 1, 1);
}

static void
GP0MonochromeOpaqueQuad(GPU* Gpu, u32 Param)
{
    float R = (float)(Param & 0xFF) / 255.0f;
    float G = (float)((Param >> 8) & 0xFF) / 255.0f;
    float B = (float)((Param >> 16) & 0xFF) / 255.0f;
    glColor3f(R, G, B);
    glBegin(GL_QUADS);
    glVertex2i((s32)(Gpu->Gp0Packets[0] & 0xFFFF), (s32)(Gpu->Gp0Packets[0] >> 16));
    glVertex2i((s32)(Gpu->Gp0Packets[1] & 0xFFFF), (s32)(Gpu->Gp0Packets[1] >> 16));
    glVertex2i((s32)(Gpu->Gp0Packets[3] & 0xFFFF), (s32)(Gpu->Gp0Packets[3] >> 16));
    glVertex2i((s32)(Gpu->Gp0Packets[2] & 0xFFFF), (s32)(Gpu->Gp0Packets[2] >> 16));
    glEnd();
    glColor4f(1, 1, 1, 1);
}

void
DMA2Write(void *Object, u32 Value)
{
    printf("DMA2 0x%08lX\n", Value);
}

static void __attribute__((constructor))
InitFuncTables()
{
    for (int i = 0; i < 0xFF; ++i)
    {
        GP0FuncTable[i] = GP0Nop;
    }

    GP0FuncTable[GP0_COMMAND_FILL_RECT] = GP0FillRect;
    GP0FuncTable[0x28] = GP0MonochromeOpaqueQuad;
}

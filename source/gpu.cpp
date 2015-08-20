
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

    GfxHandle = gfxCreateDevice(400, 240);
    gfxMakeCurrent(GfxHandle);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 320, 240, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void
GpuExecuteCommand(GPU *Gpu, u8 Command, u32 Param)
{
    if (Command == GP0_COMMAND_FILL_RECT)
    {
        printf("Draw rect\n");
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
    
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    printf("GP0 0x%08lX", Value);

    if (Command == GP0_COMMAND_FILL_RECT)
    {
        printf("Fill rect\n");
        Gpu->Gp0WaitingCmd = Value;
        Gpu->Gp0PacketsLeft = 2;
    }
}

void
GpuGp1(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    printf("GP1 0x%08lX", Value);

    if (Command == GP1_COMMAND_RST)
    {
        // TODO reset command
    }

    if (Command == GP1_COMMAND_DISP_EN)
    {
        Gpu->Status |= GPU_STAT_DISP_EN;
    }
}

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "platform.h"
#include "joypad.h"
#include <unistd.h>

#ifdef _3DS
#include <3ds.h>
#include <gfx_device.h>

static void *GfxHandle;

void
InitPlatform(int argc, char **argv)
{
    sdmcInit();
    if (argc > 1) chdir(argv[1]);
    gfxInitDefault();
    hidInit();
    consoleInit(GFX_BOTTOM, NULL);

    GfxHandle = gfxCreateDevice(240, 400, 0);
    gfxMakeCurrent(GfxHandle);
}

void
ExitPlatform()
{
    gfxExit();
    hidExit();
    sdmcExit();
}

void
SwapBuffersPlatform()
{
    gfxFlush(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 240, 400, GX_TRANSFER_FMT_RGB8);
    gfxSwapBuffersGpu();
    hidScanInput();
}

bool
MainLoopPlatform()
{
    return aptMainLoop();
}

u32
GetDigitalSwitchesPlatform()
{
    u32 Value;
    Value = 0xFFFFFFFF;

    u32 KeysHeld = hidKeysHeld() | hidKeysDown();
    if (KeysHeld & KEY_A)
    {
        Value &= ~JOY_BUTTON_CIRCLE;
    }

    if (KeysHeld & KEY_B)
    {
        Value &= ~JOY_BUTTON_CROSS;
    }

    if (KeysHeld & KEY_SELECT)
    {
        Value &= ~JOY_BUTTON_SELECT;
    }

    if (KeysHeld & KEY_START)
    {
        Value &= ~JOY_BUTTON_START;
    }

    if (KeysHeld & KEY_DRIGHT)
    {
        Value &= ~JOY_BUTTON_RIGHT;
    }

    if (KeysHeld & KEY_DLEFT)
    {
        Value &= ~JOY_BUTTON_LEFT;
    }

    if (KeysHeld & KEY_DUP)
    {
        Value &= ~JOY_BUTTON_UP;
    }

    if (KeysHeld & KEY_DDOWN)
    {
        Value &= ~JOY_BUTTON_DOWN;
    }

    if (KeysHeld & KEY_R)
    {
        Value &= ~JOY_BUTTON_R1;
    }

    if (KeysHeld & KEY_L)
    {
        Value &= ~JOY_BUTTON_L1;
    }

    if (KeysHeld & KEY_ZL)
    {
        Value &= ~JOY_BUTTON_L2;
    }

    if (KeysHeld & KEY_ZR)
    {
        Value &= ~JOY_BUTTON_R2;
    }
    return Value;
}

bool PlatformHasDebugger()
{
    return false;
}

void PlatformAttachDebugger(void *Cpu)
{

}

#else
#include <GLFW/glfw3.h>
#include <stdarg.h>
#include <cstdio>
#include <cstring>
#include "disasm.h"
#include "stb_truetype.h"
#include "crystal_font.h"

static GLFWwindow *GfxHandle;
static GLFWwindow *DebugWindow;

static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
static GLuint FontTexID;
static float SavedXPos = 0;
static float FontHeight = 12.0f;

static MIPS_R3000 *DebugCpu = nullptr;

static void
InitFont(void)
{

    unsigned char *temp_bitmap = (unsigned char *)malloc(512*512);
    stbtt_BakeFontBitmap(crystal_font,0, FontHeight, temp_bitmap,512,512, 32,96, cdata);
    glGenTextures(1, &FontTexID);
    glBindTexture(GL_TEXTURE_2D, FontTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    free(temp_bitmap);
}

static void
PrintString(const char *text)
{
    float y = 0;
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, FontTexID);
    glBegin(GL_QUADS);
    glColor3f(0, 0, 0);
    while (*text) {
        if (*text >= 32 && ((unsigned char)*text) < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512,512, *text-32, &SavedXPos,&y,&q,1);//1=opengl & d3d10+,0=d3d9
            glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
            glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
        }
        ++text;
    }
    glEnd();
    glColor3f(1, 1, 1);
}

static int
DisassemblerPrintOverride(const char *fmt, ...)
{
    const int BufLen = 1024;
    char Buffer[BufLen];
    memset(Buffer, 0, BufLen);
    va_list argp;
    va_start(argp, fmt);
    const int ret = vsnprintf(Buffer, BufLen, fmt, argp);
    PrintString(Buffer);
    va_end(argp);
    return ret;
}

void
InitPlatform(int argc, char **argv)
{
    if (argc > 1) chdir(argv[1]);
    glfwInit();

    glfwWindowHint(GLFW_DOUBLEBUFFER, false);
    DebugWindow = glfwCreateWindow(800, 480, "CTR64 Debugger", NULL, NULL);
    glfwMakeContextCurrent(DebugWindow);
    InitFont();
    GfxHandle = glfwCreateWindow(800, 480, "CTR64", NULL, NULL);
    glfwMakeContextCurrent(GfxHandle);
    glfwSwapInterval(0);
    DisassemblerSetPrintFunction(DisassemblerPrintOverride);
}

void
SwapBuffersPlatform()
{
    glfwMakeContextCurrent(DebugWindow);
    glFlush();
    glfwMakeContextCurrent(GfxHandle);
    glFlush();
    glfwSwapBuffers(GfxHandle);
    glfwSwapBuffers(DebugWindow);
    glfwPollEvents();
}

static void
DebuggerDrawCpuRegisters()
{
    glLoadIdentity();
    glTranslatef(600, 12, 0);
    for (int i = 0; i < 32; ++i)
    {
        glTranslatef(0, FontHeight, 0);
        SavedXPos = 0;
        u32 Hi = (u32)((DebugCpu->GPR[i] & 0xFFFFFFFF00000000) >> 32);
        u32 Lo = (u32)(DebugCpu->GPR[i] & 0xFFFFFFFF);
        DisassemblerPrintOverride("%s: %08lX:%08lX", DisassemblerGetGPRName(i), Hi, Lo);
    }
    glTranslatef(0, FontHeight, 0);
    SavedXPos = 0;
    u32 Hi = (u32)((DebugCpu->pc & 0xFFFFFFFF00000000) >> 32);
    u32 Lo = (u32)(DebugCpu->pc & 0xFFFFFFFF);
    DisassemblerPrintOverride("pc: %08lX:%08lX", Hi, Lo);
}

static void
PlatformDrawDebbuger()
{
    glfwMakeContextCurrent(DebugWindow);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 800, 480, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    SavedXPos = 0;
    glTranslatef(4, 12, 0);
    PrintString("Disassembly");
    for (int i = 0; i < 32; ++i)
    {
        glTranslatef(0, FontHeight, 0);
        SavedXPos = 0;
        u32 MachineCode = ReadMemWordRaw(DebugCpu, DebugCpu->pc + i * 4);
        disasm_opcode_info OpCode;
        DisassemblerDecodeOpcode(&OpCode, MachineCode, DebugCpu->pc + i * 4);
        DisassemblerPrintOpCode(&OpCode);
    }

    DebuggerDrawCpuRegisters();

    glfwMakeContextCurrent(GfxHandle);
}

bool
MainLoopPlatform()
{
    PlatformDrawDebbuger();
    return !glfwWindowShouldClose(GfxHandle);
}

void
ExitPlatform()
{
    glfwTerminate();
}

u32
GetDigitalSwitchesPlatform()
{
    u32 Value = 0;
    Value |= glfwGetKey(DebugWindow, GLFW_KEY_ENTER);
    return Value;
}

bool PlatformHasDebugger()
{
    return true;
}

#include "mips.h"

void PlatformAttachDebugger(void *Cpu)
{
    DebugCpu = (MIPS_R3000 *)Cpu;
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#endif

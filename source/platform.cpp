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
#ifdef __cplusplus
extern "C" {
#endif

#include <3ds/sdmc.h>
#include <3ds/console.h>
#include <3ds/services/hid.h>
#include <3ds/services/apt.h>

#ifdef __cplusplus
}
#endif

static void *GfxHandle;

void
InitPlatform(int argc, char **argv)
{
    sdmcInit();
    if (argc > 1) chdir(argv[1]);
    gfxInitDefault();
    hidInit();
    consoleInit(GFX_BOTTOM, NULL);

    // @TODO graphics lib init
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

    // @TODO graphics library flush to framebuffer
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
#include <string>
#include <iomanip>
#include <sstream>
#include <thread>
#include "disasm.h"
#include "stb_truetype.h"
#include "crystal_font.h"

static GLFWwindow *GfxHandle;
static GLFWwindow *DebugWindow;

static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
static GLuint FontTexID;
static float SavedXPos = 0;
static float FontHeight = 14.0f;

static MIPS_R3000 *DebugCpu = nullptr;
static struct {float r; float g; float b;} FontColor = {1, 1, 1};
static u64 DisassemblyAddress;
static u32 CyclesToStep = 1;
static std::string DebugTempStr;
enum
{
    MODE_NORMAL = 0,
    MODE_GOTO,
    MODE_CYCLES_TO_STEP
};
static u32 ModeSelect = MODE_NORMAL;

template< typename T >
std::string int_to_hex( T i )
{
    std::stringstream stream;
    stream << std::uppercase
    << std::setfill ('0') << std::setw(8)
    << std::hex << i;
    return stream.str();
}

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
    glColor3f(FontColor.r, FontColor.g, FontColor.b);
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

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (window != DebugWindow) return;

    if (ModeSelect == MODE_GOTO)
    {
        if (action != GLFW_PRESS) return;

        if ((key >= GLFW_KEY_A && key <= GLFW_KEY_F) || (key >= GLFW_KEY_0 && key <= GLFW_KEY_9))
        {
            DebugTempStr += (unsigned char)key;
        }

        if (key == GLFW_KEY_BACKSPACE)
        {
            if (DebugTempStr.length()) DebugTempStr.pop_back();
        }

        if (key == GLFW_KEY_ENTER)
        {
            ModeSelect = MODE_NORMAL;
            DisassemblyAddress = strtoul(DebugTempStr.c_str(), nullptr, 16);
        }
    }
    else if (ModeSelect == MODE_CYCLES_TO_STEP)
    {
        if (action != GLFW_PRESS) return;

        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
        {
            DebugTempStr += (unsigned char)key;
        }

        if (key == GLFW_KEY_BACKSPACE)
        {
            if (DebugTempStr.length()) DebugTempStr.pop_back();
        }

        if (key == GLFW_KEY_ENTER)
        {
            ModeSelect = MODE_NORMAL;
            CyclesToStep = strtoul(DebugTempStr.c_str(), nullptr, 10);
            if (CyclesToStep < 1) CyclesToStep = 1;
        }
    }
    else
    {
        if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            StepCpu(DebugCpu, CyclesToStep);
            DisassemblyAddress = DebugCpu->pc;
        }
        if (key == GLFW_KEY_G && action == GLFW_PRESS)
        {
            ModeSelect = MODE_GOTO;
            DebugTempStr = int_to_hex(DisassemblyAddress);
        }
        if (key == GLFW_KEY_C && action == GLFW_PRESS)
        {
            ModeSelect = MODE_CYCLES_TO_STEP;
            DebugTempStr = std::to_string(CyclesToStep);
        }
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            DisassemblyAddress += 4;
        }
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            DisassemblyAddress -= 4;
        }
        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            DebugCpu->pc = RESET_VECTOR;
        }
    }
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
    glfwSwapInterval(0);
    DisassemblerSetPrintFunction(DisassemblerPrintOverride);
    glfwSetKeyCallback(DebugWindow, KeyCallback);
}

void
SwapBuffersPlatform()
{
    glfwMakeContextCurrent(DebugWindow);
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
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    SavedXPos = 0;
    glTranslatef(4, 12, 0);
    PrintString("Disassembly");
    for (int i = -16; i < 16; ++i)
    {
        glTranslatef(0, FontHeight, 0);
        SavedXPos = 0;
        u32 MachineCode = ReadMemWordRaw(DebugCpu, DisassemblyAddress + i * 4);
        disasm_opcode_info OpCode;
        if ((DisassemblyAddress + i * 4) == DebugCpu->pc) FontColor = {1, 0, 1};
        DisassemblerDecodeOpcode(&OpCode, MachineCode, DisassemblyAddress + i * 4);
        DisassemblerPrintOpCode(&OpCode);
        if ((DisassemblyAddress + i * 4) == DebugCpu->pc) FontColor = {1, 1, 1};
    }
    glLoadIdentity();
    glTranslatef(404, 12, 0);
    for (int i = -16; i < 16; ++i)
    {
        glTranslatef(0, FontHeight, 0);
        SavedXPos = 0;
        u32 MachineCode = ReadMemWordRaw(DebugCpu, DisassemblyAddress + i * 4);
        DisassemblerPrintOverride("%08lX", MachineCode);
    }

    if (ModeSelect == MODE_GOTO)
    {
        glLoadIdentity();
        glTranslatef(150, 12, 0);
        SavedXPos = 0;
        DisassemblerPrintOverride("Go to: %s\n", DebugTempStr.c_str());
    }

    if (ModeSelect == MODE_CYCLES_TO_STEP)
    {
        glLoadIdentity();
        glTranslatef(150, 12, 0);
        SavedXPos = 0;
        DisassemblerPrintOverride("Set cycles to step: %s\n", DebugTempStr.c_str());
    }

    DebuggerDrawCpuRegisters();
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

void *
PlatformCreateThread(void (*Thread)())
{
    std::thread *Needle = new std::thread(Thread);
    return Needle;
}

void
PlatformJoinThread(void *ThreadHandle)
{
    std::thread *Needle = (std::thread *)ThreadHandle;
    if (Needle)
    {
        Needle->join();
        delete Needle;
    }
}

void
PlatformSleepThread(s64 nano)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(nano));
}

void *
PlatformGetGfxContext()
{
    return GfxHandle;
}

void
PlatformMakeContextCurrent(void *Handle)
{
    glfwMakeContextCurrent((GLFWwindow *)Handle);
}

#include "mips.h"

void PlatformAttachDebugger(void *Cpu)
{
    DebugCpu = (MIPS_R3000 *)Cpu;
    DisassemblyAddress = DebugCpu->pc;
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#endif

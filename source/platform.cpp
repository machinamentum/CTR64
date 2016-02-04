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

#else
#include <GLFW/glfw3.h>

static GLFWwindow *GfxHandle;

void
InitPlatform(int argc, char **argv)
{
    if (argc > 1) chdir(argv[1]);
    glfwInit();

    glfwWindowHint(GLFW_DOUBLEBUFFER, false);
    GfxHandle = glfwCreateWindow(800, 480, "CTRX", NULL, NULL);
    glfwMakeContextCurrent(GfxHandle);
    glfwSwapInterval(0);
}

void
SwapBuffersPlatform()
{
    glFlush();
    glfwSwapBuffers(GfxHandle);
    glfwPollEvents();
}

bool
MainLoopPlatform()
{
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
    u32 Value;
    Value = 0xFFFFFFFF;
    int count;
    const unsigned char* States = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
    for (int i = 0; i < count && i < 15; ++i)
    {
        Value &= ~((States[i] == GLFW_PRESS) << i);
    }
    return Value;
}


#endif

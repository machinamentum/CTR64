
#include "platform.h"
#include <unistd.h>

#ifdef _3DS
#include <3ds.h>
#include <gfx_device.h>

static void *GfxHandle;

void
InitPlatform(int argc, char **argv)
{
    if (argc > 1) chdir(argv[1]);
    gfxInitDefault();
    hidInit(NULL);
    PrintConsole BottomConsole;
    consoleInit(GFX_BOTTOM, &BottomConsole);

    GfxHandle = gfxCreateDevice(240, 400);
    gfxMakeCurrent(GfxHandle);
}

void
ExitPlatform()
{
    gfxExit();
    hidExit();
}

void
SwapBuffersPlatform()
{
    gfxFlush(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL));
    gfxFlushBuffers();
    gfxSwapBuffersGpu();
    gspWaitForVBlank();
}

bool
MainLoopPlatform()
{
    return aptMainLoop();
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
    GfxHandle = glfwCreateWindow(400, 240, "CTRX", NULL, NULL);
    glfwMakeContextCurrent(GfxHandle);
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

void ExitPlatform()
{
    glfwTerminate();
}

#endif

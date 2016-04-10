/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "vi.h"
#ifdef _3DS
#include <GL/gl.h>
#else
#include <GLFW/glfw3.h>
#endif
#include <thread>

static VideoInterface *VI;
static MIPS_R3000 *Cpu;
static bool KeepThreadActive;
static void *ThreadHandle;

static void
VIUpdateRoutine()
{
    void *GfxHandle = PlatformGetGfxContext();
    PlatformMakeContextCurrent(GfxHandle);
    GLuint MainWindowTex;
    glGenTextures(1, &MainWindowTex);
    glBindTexture(GL_TEXTURE_2D, MainWindowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    while (KeepThreadActive)
    {
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, MapVirtualAddress(Cpu, VI->DRAMAddr));
        glLoadIdentity();
        glTranslatef(-0.75f, -0.75f, 0);
        glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glTexCoord2f(0, 0);
            glVertex2f(0, 1);
            glTexCoord2f(0, 1);
            glVertex2f(1, 1);
            glTexCoord2f(1, 1);
            glVertex2f(1, 0);
            glTexCoord2f(1, 0);
        glEnd();

        glFlush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void
VIStartThread(MIPS_R3000 *C, VideoInterface *V)
{
    VI = V;
    Cpu = C;
    KeepThreadActive = true;
    ThreadHandle = PlatformCreateThread(VIUpdateRoutine);
}

void
VICloseThread()
{
    KeepThreadActive = false;
    PlatformJoinThread(ThreadHandle);
}

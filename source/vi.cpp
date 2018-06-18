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
// @TODO include graphics lib header
#else
#include <GLFW/glfw3.h>
#endif

static VideoInterface *VI;
static MIPS_R3000 *Cpu;
static bool KeepThreadActive;
static void *ThreadHandle;
#include <cstdio>
#include <cstring>

// @FixMe @Refactor we're no longer going to rely solely on OpenGL for graphics rendering. Replace with jrh3d or other, and factor GL code into desktop/platform stuff.
// @FixMe @Refactor we're no longer going to rely solely on OpenGL for graphics rendering. Replace with jrh3d or other, and factor GL code into desktop/platform stuff.
// @FixMe @Refactor we're no longer going to rely solely on OpenGL for graphics rendering. Replace with jrh3d or other, and factor GL code into desktop/platform stuff.
// @FixMe @Refactor we're no longer going to rely solely on OpenGL for graphics rendering. Replace with jrh3d or other, and factor GL code into desktop/platform stuff.
// @FixMe @Refactor we're no longer going to rely solely on OpenGL for graphics rendering. Replace with jrh3d or other, and factor GL code into desktop/platform stuff.


static void
VIUpdateRoutine()
{
    Mutex *Mtx = PlatformGetGfxMutex();
    void *GfxHandle = PlatformGetGfxContext();
    #ifndef _3DS
    PlatformLockMutex(Mtx);
    PlatformMakeContextCurrent(GfxHandle);
    GLuint MainWindowTex;
    glGenTextures(1, &MainWindowTex);
    glBindTexture(GL_TEXTURE_2D, MainWindowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    PlatformUnLockMutex(Mtx);
    while (KeepThreadActive)
    {
        PlatformLockMutex(Mtx);
        PlatformMakeContextCurrent(GfxHandle);

        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        void *Addr = MapVirtualAddress(Cpu, __builtin_bswap32(VI->DRAMAddr), MEM_REGION_READ);
        if (!Addr) {
            printf("Could not map DRAMAddr at %llX\n", VI->DRAMAddr);
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, Addr);
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
        PlatformUnLockMutex(Mtx);
        PlatformSleepThread(PLATFORM_SLEEP_MILLISECONDS(16));
    }
    #endif
}


void
VIStartThread(MIPS_R3000 *C, VideoInterface *V)
{
    VI = V;
    Cpu = C;
    KeepThreadActive = true;
    memset(V, 0, sizeof(VideoInterface));
    ThreadHandle = PlatformCreateThread(VIUpdateRoutine);
}

void
VICloseThread()
{
    KeepThreadActive = false;
    PlatformJoinThread(ThreadHandle);
}

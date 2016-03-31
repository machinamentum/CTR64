/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _3DS
#include <3ds.h>
#include <3ds/types.h>
#else
#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

#include <cstdlib>
#define linearAlloc malloc
#define linearFree free
#endif

void InitPlatform(int, char **);
void ExitPlatform();
void SwapBuffersPlatform();
bool MainLoopPlatform();
u32 GetDigitalSwitchesPlatform();
bool PlatformHasDebugger();
void PlatformAttachDebugger(void *);

#endif

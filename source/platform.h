
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
#endif

void InitPlatform(int, char **);
void ExitPlatform();
void SwapBuffersPlatform();
bool MainLoopPlatform();

#endif

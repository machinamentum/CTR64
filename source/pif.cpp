/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */

#include "pif.h"
#include <cstdio>

static PIFConfig *Config;
static bool KeepThreadActive;
static void *ThreadHandle;
static u8 ReqStatus[3] = {0x05, 0x00, 0xFF};

static void
PIFUpdateRoutine()
{
    while (KeepThreadActive)
    {
        u8 *RAM = Config->RAM;
        u8 Status = RAM[0x3F];
        if (Status & 0x20)
        {
            RAM[0x3F] |= PIF_BUSY_BIT;
        }
        if (Status == 1)
        {
            RAM[0x3F] = PIF_BUSY_BIT;
            Config->ChannelCounter = 0;
            for (u32 i = 0; i < 0x3F; ++i)
            {
                s8 TxBytes = RAM[i];
                if ((u8)TxBytes == 0xFE) break;
                if (TxBytes < 0) continue;
                if (TxBytes == 0)
                {
                    ++Config->ChannelCounter;
                    continue;
                }
                u8 RxBytes = RAM[++i] & 0x3F;
                u32 RBIndex = i;
                u8 Command = RAM[++i];
                switch (Command)
                {
                    case PIF_COMMAND_REQUEST_STATUS:
                    {
                        if (RxBytes > 3)
                        {
                            RxBytes = 3;
                            RAM[RBIndex] |= PIF_ERROR_BAD_TRANSFER_SIZE;
                        }
                        if (TxBytes > 1)
                        {
                            RAM[RBIndex] |= PIF_ERROR_BAD_TRANSFER_SIZE;
                        }
                        for (u32 e = 0; e < RxBytes; ++e)
                        {
                            RAM[++i] = ((e == 2) ? 0x02 : ReqStatus[e]);
                        }
                    }

                    default:
                    {
                        printf("Unkown PIF command %02X\n", Command);
                        i += RxBytes;
                    }
                }
                ++Config->ChannelCounter;

            }

            RAM[0x3F] = 0;
        }

        PlatformSleepThread(PLATFORM_SLEEP_MILLISECONDS(16));
    }
}

void
PIFStartThread(PIFConfig *C)
{
    Config = C;
    KeepThreadActive = true;
    ThreadHandle = PlatformCreateThread(PIFUpdateRoutine);
}

void
PIFCloseThread()
{
    KeepThreadActive = false;
    PlatformJoinThread(ThreadHandle);
}

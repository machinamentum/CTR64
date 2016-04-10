/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef PIF_H
#define PIF_H

#include "platform.h"

#define PIF_COMMAND_REQUEST_STATUS  (0x00)
#define PIF_COMMAND_READ_BUTTONS    (0x01)
#define PIF_COMMAND_READ_MEMPACK    (0x02)
#define PIF_COMMAND_WRITE_MEMPACK   (0x03)
#define PIF_COMMAND_READ_EEPROM     (0x04)
#define PIF_COMMAND_WRITE_EEPROM    (0x05)
#define PIF_COMMAND_RESET           (0xFF)

#define PIF_ERROR_NONE              (0x00)
#define PIF_ERROR_BAD_TRANSFER_SIZE (0x40)
#define PIF_ERROR_MISSING_DEVICE    (0x80)

struct PIFConfig
{
    u8 *RAM;
    u8 *CartEEPROM;
    u8 ChannelCounter;
};

void PIFStartThread(PIFConfig *Config);
void PIFCloseThread();


#endif

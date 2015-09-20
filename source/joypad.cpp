/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "joypad.h"

static u32 RXRead;
static u32 JoySequenceState = 0;
const static u32 JoyID = JOY_ID_DIGITAL_PAD;
static u32 DigitalSwitches = 0;

void
JoyTxWrite(void *Object, u32 Value)
{
    if (!JoySequenceState)
    {
        if (Value == 0x01)
        {
            RXRead = 0xFF;
            ++JoySequenceState;
            return;
        }
    }

    if (JoySequenceState == 1)
    {
        if (Value == 0x42)
        {
            RXRead = JoyID & 0xFF;
            ++JoySequenceState;
            return;
        }
    }

    if (JoySequenceState == 2)
    {
        if (Value == 0)
        {
            RXRead = (JoyID >> 8) & 0xFF;
            ++JoySequenceState;
            return;
        }
    }

    if (JoySequenceState == 3)
    {
        DigitalSwitches = GetDigitalSwitchesPlatform();
        RXRead = DigitalSwitches & 0xFF;
        ++JoySequenceState;
        return;
    }

    if (JoySequenceState == 4)
    {
        RXRead = (DigitalSwitches >> 8) & 0xFF;
        JoySequenceState = 0;
        return;
    }
}

u32
JoyRxRead(void *Object, u32 Address)
{
    return RXRead;
}

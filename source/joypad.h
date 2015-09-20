/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef JOYPAD_H
#define JOYPAD_H

#include "platform.h"

#define JOY_TX_DATA (0x1F801040)
#define JOY_RX_DATA (0x1F801040)
#define JOY_STAT    (0x1F801044)
#define JOY_MODE    (0x1F801048)
#define JOY_CTRL    (0x1F80104A)
#define JOY_BAUD    (0x1F80104E)

#define JOY_ID_DIGITAL_PAD   (0x5A41)
#define JOY_ID_ANALOG_STICK  (0x5A53)

#define JOY_BUTTON_SELECT     (1 << 0)
#define JOY_BUTTON_L3         (1 << 1)
#define JOY_BUTTON_R3         (1 << 2)
#define JOY_BUTTON_START      (1 << 3)
#define JOY_BUTTON_UP         (1 << 4)
#define JOY_BUTTON_RIGHT      (1 << 5)
#define JOY_BUTTON_DOWN       (1 << 6)
#define JOY_BUTTON_LEFT       (1 << 7)
#define JOY_BUTTON_L2         (1 << 8)
#define JOY_BUTTON_R2         (1 << 9)
#define JOY_BUTTON_L1         (1 << 10)
#define JOY_BUTTON_R1         (1 << 11)
#define JOY_BUTTON_TRIANGLE   (1 << 12)
#define JOY_BUTTON_CIRCLE     (1 << 13)
#define JOY_BUTTON_CROSS      (1 << 14)
#define JOY_BUTTON_SQUARE     (1 << 15)


void JoyTxWrite(void *, u32);
u32 JoyRxRead(void *, u32);

#endif

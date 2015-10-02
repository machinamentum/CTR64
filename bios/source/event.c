/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

typedef struct
{
    int Class;
    int Spec;
    int Mode;
    void (*Callback)();
    int Enabled;
} EvCB;

static EvCB EventTable[0x10];
static int FreeEventIndex = 0;

EvCB *
get_free_EvCB_slot()
{
    printf("%s\n", __FUNCTION__);
    return &EventTable[FreeEventIndex++];
}

int
OpenEvent(int Class, int Spec, int Mode, void (*Callback)())
{
    printf("%s\n", __FUNCTION__);
    printf("Event Class: 0x%X\n", Class);
    printf("Event Spec:  0x%X\n", Spec);
    printf("Event Mode:  0x%X\n", Mode);
    printf("Event Func:  0x%X\n", (int)Callback);

    EvCB *Event = get_free_EvCB_slot();
    Event->Class = Class;
    Event->Spec = Spec;
    Event->Mode = Mode;
    Event->Callback = Callback;
    Event->Enabled = 0;
    return 0xF1000000 | (FreeEventIndex - 1);
}

int
CloseEvent(int EventID)
{
    printf("%s\n", __FUNCTION__);
    return 1;
}

int
EnableEvent(int EventID)
{
    printf("%s\n", __FUNCTION__);
    printf("Event 0x%X\n", EventID);
    int Index = EventID & 0xF;
    EvCB *Event = &EventTable[Index];
    Event->Enabled = 1;
    return 1;
}

int
DisableEvent(void *Event)
{
    printf("%s\n", __FUNCTION__);
    return 1;
}

int
WaitEvent(void *Event)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
TestEvent(void *Event)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

void
DeliverEvent(int Class, int Spec)
{
    for (int i = 0; i < 0x10; ++i)
    {
        EvCB *Event = &EventTable[i];
        if (Event->Enabled)
        {
            if ((Event->Class == Class) && (Event->Spec == Spec))
            {
                if (Event->Mode == 0x2000)
                {
                    // TODO busy flag?
                }
                else if (Event->Mode == 0x1000)
                {
                    Event->Callback();
                }
            }
        }
    }
}

void
UnDeliverEvent(int Class, int Spec)
{
    printf("%s\n", __FUNCTION__);
}

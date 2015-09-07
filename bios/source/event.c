/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

void *
get_free_EvCB_slot()
{
    return 0;
}

void
OpenEvent(int Class, int Spec, int Mode, void (*Callback)())
{

}

int
CloseEvent(void *Event)
{
    return 1;
}

int
EnableEvent(void *Event)
{
    return 1;
}

int
DisableEvent(void *Event)
{
    return 1;
}

int
WaitEvent(void *Event)
{
    return 0;
}

int
TestEvent(void *Event)
{
    return 0;
}

void
DeliverEvent(int Class, int Spec)
{

}

void
UnDeliverEvent(int Class, int Spec)
{

}
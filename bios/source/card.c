/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

void
InitCard(int PadEnable)
{
    printf("%s\n", __FUNCTION__);
}

void
StartCard()
{
    printf("%s\n", __FUNCTION__);
}

void
StopCard()
{
    printf("%s\n", __FUNCTION__);
}

void
_bu_init()
{
    printf("%s\n", __FUNCTION__);
}

void
_card_info_subfunc(int Port)
{
    printf("%s\n", __FUNCTION__);
}

void
_card_info(int Port)
{
    printf("%s\n", __FUNCTION__);
}

void
card_write_test(int Port)
{
    printf("%s\n", __FUNCTION__);
}

void
allow_new_card()
{
    printf("%s\n", __FUNCTION__);
}

void
write_card_sector(int Port, int Sector, void *Src)
{
    printf("%s\n", __FUNCTION__);
}

void
read_card_sector(int Port, int Sector, void *Dst)
{
    printf("%s\n", __FUNCTION__);
}

int
get_card_status(int Slot)
{
    printf("%s\n", __FUNCTION__);
    return 0x11;
}

int
wait_card_status(int Slot)
{
    printf("%s\n", __FUNCTION__);
    return 0x11;
}

void
bu_callback_okay()
{
    printf("%s\n", __FUNCTION__);
}

void
bu_callback_err_write()
{
    printf("%s\n", __FUNCTION__);
}

void
bu_callback_err_busy()
{
    printf("%s\n", __FUNCTION__);
}

void
bu_callback_err_eject()
{
    printf("%s\n", __FUNCTION__);
}

void
bu_callback_err_prev_write()
{
    printf("%s\n", __FUNCTION__);
}

int
get_bu_callback_port()
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

void
_card_async_load_directory(int Port)
{
    printf("%s\n", __FUNCTION__);
}

void
set_card_auto_format(int Flag)
{
    printf("%s\n", __FUNCTION__);
}

static int CardFindMode = 0;

void
set_card_find_mode(int Mode)
{
    printf("%s\n", __FUNCTION__);
    CardFindMode = Mode;
}

int
get_card_find_mode()
{
    printf("%s\n", __FUNCTION__);
    return CardFindMode;
}

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

}

void
StartCard()
{

}

void
StopCard()
{

}

void
_bu_init()
{

}

void
_card_info_subfunc(int Port)
{

}

void
_card_info(int Port)
{

}

void
card_write_test(int Port)
{

}

void
allow_new_card()
{

}

void
write_card_sector(int Port, int Sector, void *Src)
{

}

void
read_card_sector(int Port, int Sector, void *Dst)
{

}

int
get_card_status(int Slot)
{
    return 0x11;
}

int
wait_card_status(int Slot)
{
    return 0x11;
}

void
bu_callback_okay()
{

}

void
bu_callback_err_write()
{

}

void
bu_callback_err_busy()
{

}

void
bu_callback_err_eject()
{

}

void
bu_callback_err_prev_write()
{

}

int
get_bu_callback_port()
{
    return -1;
}

void
_card_async_load_directory(int Port)
{

}

void
set_card_auto_format(int Flag)
{

}

static int CardFindMode = 0;

void
set_card_find_mode(int Mode)
{
    CardFindMode = Mode;
}

int
get_card_find_mode()
{
    return CardFindMode;
}
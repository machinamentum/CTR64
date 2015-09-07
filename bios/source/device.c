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
tty_cdevinput(int Circ, int Char)
{

}

void
tty_cdevscan()
{

}

void
tty_circgetc(int Circ)
{

}

void
tty_circputc(int Char, int Circ)
{
    
}

void
dev_tty_init()
{

}

void
dev_tty_open(int Fcb, const char *Path, int AccessMode)
{

}

void
dev_tty_in_out(int Fcb, int Cmd)
{

}

void
dev_tty_ioctl(int Fcb, int Cmd, int Arg)
{

}

void
dev_cd_open(int Fcb, const char *Path, int AccessMode)
{

}

void
dev_cd_read(int Fcb, void *Dst, int Length)
{

}

void
dev_cd_close(int Fcd)
{

}

void
dev_cd_firstfile(int Fcb, const char *Path, DirEntry *Entry)
{

}

void
dev_cd_nextfile(int Fcb, DirEntry *Entry)
{

}

void
dev_cd_chdir(int Fcb, const char *Path)
{

}

void
dev_card_open(int Fcb, const char *Path, int AccessMode)
{

}

void
dev_card_read(int Fcb, void *Dst, int Length)
{

}

void
dev_card_write(int Fcb, void *Src, int Length)
{

}

void
dev_card_close(int Fcd)
{

}

void
dev_card_firstfile(int Fcb, const char *Path, DirEntry *Entry)
{

}

void
dev_card_nextfile(int Fcb, DirEntry *Entry)
{

}

void
dev_card_erase(int Fcb, const char *Path)
{

}

void
dev_card_undelete(int Fcb, const char *Path)
{

}

void
dev_card_format(int Fcb)
{

}

void
dev_card_rename(int Fcb1, const char *Old, int Fcb2, const char *New)
{

}

void
testdevice(int Fcb, const char *Path)
{

}

void
AddDevice(void *DeviceInfo)
{

}

void
AddCDROMDevice()
{

}

void
AddMemCardDevice()
{

}

void
AddDuartTtyDevice()
{

}

void
AddDummyTtyDevice()
{

}

void
RemoveDevice(const char *DeviceName)
{

}

void
InstallDevices(int TtyFlag)
{
    // TODO FCB and DCB
    AddCDROMDevice();
    AddMemCardDevice();
    KernelRedirect(TtyFlag);
}

void
PrintInstalledDevices()
{

}


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
    printf("%s\n", __FUNCTION__);
}

void
tty_cdevscan()
{
    printf("%s\n", __FUNCTION__);
}

void
tty_circgetc(int Circ)
{
    printf("%s\n", __FUNCTION__);
}

void
tty_circputc(int Char, int Circ)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_tty_init()
{
    printf("%s\n", __FUNCTION__);
}

void
dev_tty_open(int Fcb, const char *Path, int AccessMode)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_tty_in_out(int Fcb, int Cmd)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_tty_ioctl(int Fcb, int Cmd, int Arg)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_cd_open(int Fcb, const char *Path, int AccessMode)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_cd_read(int Fcb, void *Dst, int Length)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_cd_close(int Fcd)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_cd_firstfile(int Fcb, const char *Path, DirEntry *Entry)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_cd_nextfile(int Fcb, DirEntry *Entry)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_cd_chdir(int Fcb, const char *Path)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_open(int Fcb, const char *Path, int AccessMode)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_read(int Fcb, void *Dst, int Length)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_write(int Fcb, void *Src, int Length)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_close(int Fcd)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_firstfile(int Fcb, const char *Path, DirEntry *Entry)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_nextfile(int Fcb, DirEntry *Entry)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_erase(int Fcb, const char *Path)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_undelete(int Fcb, const char *Path)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_format(int Fcb)
{
    printf("%s\n", __FUNCTION__);
}

void
dev_card_rename(int Fcb1, const char *Old, int Fcb2, const char *New)
{
    printf("%s\n", __FUNCTION__);
}

void
testdevice(int Fcb, const char *Path)
{
    printf("%s\n", __FUNCTION__);
}

void
AddDevice(void *DeviceInfo)
{
    printf("%s\n", __FUNCTION__);
}

void
AddCDROMDevice()
{
    printf("%s\n", __FUNCTION__);
}

void
AddMemCardDevice()
{
    printf("%s\n", __FUNCTION__);
}

void
AddDuartTtyDevice()
{
    printf("%s\n", __FUNCTION__);
}

void
AddDummyTtyDevice()
{
    printf("%s\n", __FUNCTION__);
}

void
RemoveDevice(const char *DeviceName)
{
    printf("%s\n", __FUNCTION__);
}

void
InstallDevices(int TtyFlag)
{
    printf("%s\n", __FUNCTION__);
    // TODO FCB and DCB
    AddCDROMDevice();
    AddMemCardDevice();
    KernelRedirect(TtyFlag);
}

void
PrintInstalledDevices()
{
    printf("%s\n", __FUNCTION__);
}


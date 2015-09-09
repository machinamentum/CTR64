/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

int
FileOpen(const char *FileName, int AccessMode)
{
    printf(__FUNCTION__);
    return -1;
}

int
FileSeek(int Fd, int Offset, int SeekType)
{
    printf(__FUNCTION__);
    return -1;
}

int
FileRead(int Fd, void *Dst, int Length)
{
    printf(__FUNCTION__);
    return -1;
}

int
FileWrite(int Fd, void *Src, int Length)
{
    printf(__FUNCTION__);
    return 0;
}

int
FileClose(int Fd)
{
    printf(__FUNCTION__);
    return -1;
}

int
FileGetc(int Fd)
{
    printf(__FUNCTION__);
    int Result;
    char Temp;
    Result = FileRead(Fd, &Temp, 1);
    if (Result == -1) return Result;
    return Temp;
}

int
FilePutc(char Char, int Fd)
{
    printf(__FUNCTION__);
    char Temp = Char;
    return FileWrite(Fd, &Temp, 1);
}

int
chdir(const char *Name)
{
    printf(__FUNCTION__);
    return 0;
}


int
FileIoctl(int Fd, int Cmd, ...)
{
    printf(__FUNCTION__);
    return -1;
}

int
FileGetDeviceFlag(int Fd)
{
    printf(__FUNCTION__);
    return 0;
}

int
FileRename(const char *Old, const char *New)
{
    printf(__FUNCTION__);
    return 0;
}

int
FileDelete(const char *FileName)
{
    printf(__FUNCTION__);
    return 0;
}

int
FileUndelete(const char *FileName)
{
    printf(__FUNCTION__);
    return 0;
}

int
FormatDevice(const char *DeviceName)
{
    printf(__FUNCTION__);
    return 0;
}

int
GetLastFileError(int Fd)
{
    printf(__FUNCTION__);
    return 0xFFFFFFFF;
}

DirEntry *
firstfile(const char *FileName, DirEntry *Entry)
{
    printf(__FUNCTION__);
    return 0;
}

DirEntry *
nextfile(DirEntry *Entry)
{
    printf(__FUNCTION__);
    return 0;
}

int
GetLastError()
{
    printf(__FUNCTION__);
    return 0;
}

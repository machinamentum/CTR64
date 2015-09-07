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
    return -1;
}

int
FileSeek(int Fd, int Offset, int SeekType)
{
    return -1;
}

int
FileRead(int Fd, void *Dst, int Length)
{
    return -1;
}

int
FileWrite(int Fd, void *Src, int Length)
{
    return 0;
}

int
FileClose(int Fd)
{
    return -1;
}

int
FileGetc(int Fd)
{
    int Result;
    char Temp;
    Result = FileRead(Fd, &Temp, 1);
    if (Result == -1) return Result;
    return Temp;
}

int
FilePutc(char Char, int Fd)
{
    char Temp = Char;
    return FileWrite(Fd, &Temp, 1);
}

int
chdir(const char *Name)
{
    return 0;
}


int
FileIoctl(int Fd, int Cmd, ...)
{
    return -1;
}

int
FileGetDeviceFlag(int Fd)
{
    return 0;
}

int
FileRename(const char *Old, const char *New)
{
    return 0;
}

int
FileDelete(const char *FileName)
{
    return 0;
}

int
FileUndelete(const char *FileName)
{
    return 0;
}

int
FormatDevice(const char *DeviceName)
{
    return 0;
}

int
GetLastFileError(int Fd)
{
    return 0xFFFFFFFF;
}

DirEntry *
firstfile(const char *FileName, DirEntry *Entry)
{
    return 0;
}

DirEntry *
nextfile(DirEntry *Entry)
{
    return 0;
}

int
GetLastError()
{
    return 0;
}

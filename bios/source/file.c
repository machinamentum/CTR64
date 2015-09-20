/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "kernel.h"

static int
_CTRX_FileOpen(const char *FileName)
{
    int *FOpenRegister = (int *)0x1F802068;
    *FOpenRegister = (int)FileName;
    return *FOpenRegister;
}

static int
_CTRX_FileRead(int Fd, void *Dst, int Length)
{
    struct
    {
        int Fd;
        void *Dst;
        int Length;
    } FReadInfo;

    FReadInfo.Fd = Fd;
    FReadInfo.Dst = Dst;
    FReadInfo.Length = Length;

    int *FReadRegister = (int *)0x1F802070;
    *FReadRegister = (int)&FReadInfo;
    return *FReadRegister;
}

static int
_CTRX_FileSeek(int Fd, int Offset, int SeekType)
{
    struct
    {
        int Fd;
        int Offset;
        int SeekType;
    } FSeekInfo;

    FSeekInfo.Fd = Fd;
    FSeekInfo.Offset = Offset;
    FSeekInfo.SeekType = SeekType;

    int *FSeekRegister = (int *)0x1F802074;
    *FSeekRegister = (int)&FSeekInfo;
    return *FSeekRegister;
}

static DirEntry *
_CTRX_firstfile(const char *FileName, DirEntry *Entry)
{
    struct
    {
        char *FileName;
        DirEntry *Entry;
    } FFInfo;

    FFInfo.FileName = (char *)FileName;
    FFInfo.Entry = Entry;

    int *FFRegister = (int *)0x1F802078;
    *FFRegister = (int)&FFInfo;
    return (DirEntry *)*FFRegister;
}

int
FileOpen(const char *FileName, int AccessMode)
{
    printf("%s\n", __FUNCTION__);
    if (memcmp(FileName, "cdrom:", strlen("cdrom:")) == 0)
    {
        printf("Opening file from cdrom\n");
        return _CTRX_FileOpen(FileName + strlen("cdrom:"));
    }
    return -1;
}

int
FileSeek(int Fd, int Offset, int SeekType)
{
    printf("%s\n", __FUNCTION__);
    return _CTRX_FileSeek(Fd, Offset, SeekType);
}

int
FileRead(int Fd, void *Dst, int Length)
{
    printf("%s\n", __FUNCTION__);
    return _CTRX_FileRead(Fd, Dst, Length);
}

int
FileWrite(int Fd, void *Src, int Length)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
FileClose(int Fd)
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

int
FileGetc(int Fd)
{
    printf("%s\n", __FUNCTION__);
    int Result;
    char Temp;
    Result = FileRead(Fd, &Temp, 1);
    if (Result == -1) return Result;
    return Temp;
}

int
FilePutc(char Char, int Fd)
{
    printf("%s\n", __FUNCTION__);
    char Temp = Char;
    return FileWrite(Fd, &Temp, 1);
}

int
chdir(const char *Name)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}


int
FileIoctl(int Fd, int Cmd, ...)
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

int
FileGetDeviceFlag(int Fd)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
FileRename(const char *Old, const char *New)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
FileDelete(const char *FileName)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
FileUndelete(const char *FileName)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
FormatDevice(const char *DeviceName)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
GetLastFileError(int Fd)
{
    printf("%s\n", __FUNCTION__);
    return 0xFFFFFFFF;
}

DirEntry *
firstfile(const char *FileName, DirEntry *Entry)
{
    printf("%s\n", __FUNCTION__);
    _CTRX_firstfile(FileName, Entry);
    printf("Entry name: %s\n", Entry->FileName);
    printf("Entry Size %X\n", Entry->Size);
    return Entry;
}

DirEntry *
nextfile(DirEntry *Entry)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

int
GetLastError()
{
    printf("%s\n", __FUNCTION__);
    return 0;
}

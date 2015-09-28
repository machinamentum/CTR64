/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <cstring>
#include "hsf.h"

void
HsfOpen(hsf *HSF, const char *FileName)
{
    HSF->FileStream = fopen(FileName, "rb");
    HSF->PVD = HsfGetPrimaryVolumeDescriptor(HSF);
}

void
HsfClose(hsf *HSF)
{
    fclose(HSF->FileStream);
    linearFree(HSF->PVD);
}

void
HsfGetSector(hsf *HSF, u32 Sector, void *Buffer)
{
    fseek(HSF->FileStream, Sector * HSF_SECTOR_SIZE, SEEK_SET);
    fread(Buffer, HSF_SECTOR_SIZE, 1, HSF->FileStream);
}

void *
HsfGetSector(hsf *HSF, u32 Sector)
{
    void *Buffer = linearAlloc(HSF_SECTOR_SIZE);
    HsfGetSector(HSF, Sector, Buffer);
    return Buffer;
}

hsf_primary_volume_descriptor *
HsfGetPrimaryVolumeDescriptor(hsf *HSF)
{
    void *Buffer = linearAlloc(HSF_SECTOR_SIZE);
    HsfGetSector(HSF, 0x10, Buffer);
    return (hsf_primary_volume_descriptor *)Buffer;
}

hsf_directory_entry *
HsfGetDirectoryEntry(hsf *HSF, const char *FileName)
{
    void *Buffer = HsfGetSector(HSF, HSF->PVD->RootDirectoryEntry.DataLocationLE);
    hsf_directory_entry *RE = (hsf_directory_entry *)Buffer;

    int IndexCurrent = 0;
    int IndexMax = RE->DataLengthLE;

    while (IndexCurrent < IndexMax)
    {
        if (strncmp(RE->FileName, FileName, strlen(FileName)) == 0)
        {
            memmove(Buffer, RE, RE->Length);
            return (hsf_directory_entry *)Buffer;
        }

        IndexCurrent += RE->Length;
        RE = (hsf_directory_entry *)((u8 *)RE + RE->Length);
    }

    linearFree(Buffer);
    return nullptr;
}

hsf_file *
HsfFileOpen(hsf *HSF, const char *FileName)
{
    hsf_file *File = (hsf_file *)linearAlloc(sizeof(hsf_file));
    File->HSF = HSF;
    File->DirectoryEntry = HsfGetDirectoryEntry(HSF, FileName);
    File->SeekPosition = 0;

    if (!File->DirectoryEntry) // File not found
    {
        linearFree(File);
        return nullptr;
    }

    return File;
}

void
HsfFileClose(hsf_file *File)
{
    linearFree(File->DirectoryEntry);
    linearFree(File);
}

void
HsfFileSeek(hsf_file *File, u32 Offset, int SeekType)
{
    if (SeekType == SEEK_SET)
    {
        File->SeekPosition = Offset;
    }
    else if (SeekType == SEEK_CUR)
    {
        File->SeekPosition += Offset;
    }
    else if (SeekType == SEEK_END)
    {
        File->SeekPosition = File->DirectoryEntry->DataLengthLE - Offset;
    }
}

int
HsfFileTell(hsf_file *File)
{
    return File->SeekPosition;
}

void
HsfFileRead(void *Buffer, size_t Size, size_t Count, hsf_file *File)
{
    size_t FinalOffset = File->SeekPosition + File->DirectoryEntry->DataLocationLE * HSF_SECTOR_SIZE;
    fseek(File->HSF->FileStream, FinalOffset, SEEK_SET);
    fread(Buffer, Size, Count, File->HSF->FileStream);
    File->SeekPosition += Size * Count;
}

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef HSF_H
#define HSF_H

#include "platform.h"
#include <cstdio>

#define HSF_SECTOR_SIZE 2048

#define HSF_VD_ID ("CD001")

#define HSF_VD_TYPE_BR      (0)
#define HSF_VD_TYPE_PVD     (1)
#define HSF_VD_TYPE_SVD     (2)
#define HSF_VD_TYPE_VPD     (3)
#define HSF_VD_TYPE_VDST  (255)

struct __attribute__((packed)) hsf_time_stamp
{
    u8 Years; //since 1900
    u8 Month;
    u8 Day;
    u8 Hour;
    u8 Minute;
    u8 Second;
    u8 GMTOffset; //15 min intervals
};

struct __attribute__((packed)) hsf_directory_entry
{
    u8 Length;
    u8 EARLength;
    u32 DataLocationLE;
    u32 DataLocationBE;
    u32 DataLengthLE;
    u32 DataLengthBE;
    hsf_time_stamp TimeStamp;
    u8 FileFlags;
    u8 FileUnitSizeInterleaved;
    u8 InterleaveGap;
    u16 VolumeSequenceNumberLE;
    u16 VolumeSequenceNumberBE;
    u8 FileNameLength;
    char FileName[1];
};

struct __attribute__((packed)) hsf_date
{
    char Year[4];
    char Month[2];
    char Day[2];
    char Hour[2];
    char Minute[2];
    char Second[2];
    char HundrethsOfSecond[2];
    u8 GMTOffset;
};

struct __attribute__((packed)) hsf_volume_descriptor
{
    u8 Type;
    char Id[5];
    u8 Version;
    u8 Data[0];
};

struct __attribute__((packed)) hsf_primary_volume_descriptor : public hsf_volume_descriptor
{
    u8 Unused0;
    char SystemIdentifier[32];
    char VolumeIdentifier[32];
    u8 Unused1[8];
    u32 VolumeSpaceSizeLE;
    u32 VolumeSpaceSizeBE;
    u8 Unused2[32];
    u16 VolumeSetSizeLE;
    u16 VolumeSetSizeBE;
    u16 VolumeSequenceNumberLE;
    u16 VolumeSequenceNumberBE;
    u16 LogicalBlockSizeLE;
    u16 LogicalBlockSizeBE;
    u32 PathTableSizeLE;
    u32 PathTableSizeBE;
    u32 PathTableLocationLE;
    u32 OptionalPathTableLocationLE;
    u32 PathTableLocationBE;
    u32 OptionalPathTableLocationBE;
    hsf_directory_entry RootDirectoryEntry;
    char VolumeSetIdentifier[128];
    char PublisherIdentifier[128];
    char DataPreparerIdentifier[128];
    char ApplicationIdentifier[128];
    char CopyrightFileIdentifier[38];
    char AbstractFileIdentifier[36];
    char BibliographicFileIdentifier[37];
    hsf_date VolumeCreationDate;
    hsf_date VolumeModificationDate;
    hsf_date VolumeExpirationDate;
    hsf_date VolumeEffectiveDate;
    u8 FileStructureVersion;
    u8 Unused3;
    u8 ApplicationUsed[512];
    u8 Reserved[653];
};

struct __attribute__((packed)) hsf_path_table_entry
{
    u8 IdentifierLength;
    u8 EARLength;
    u32 ExtentLocation;
    u16 ParentDirectoryIndex;
    char Identifier[1];
};

struct hsf
{
    FILE *FileStream;
    hsf_primary_volume_descriptor *PVD;
};

struct hsf_file
{
    hsf *HSF;
    hsf_directory_entry *DirectoryEntry;
    u32 SeekPosition;
};

void HsfOpen(hsf *HSF, const char *FileName);
void HsfClose(hsf *HSF);

void *HsfGetSector(hsf *HSF, u32 Sector);
void HsfGetSector(hsf *HSF, u32 Sector, void *Buffer);
hsf_primary_volume_descriptor *HsfGetPrimaryVolumeDescriptor(hsf *HSF);
hsf_directory_entry *HsfGetDirectoryEntry(hsf *HSF, const char *FileName);

hsf_file *HsfFileOpen(hsf *HSF, const char *FileName);
void HsfFileClose(hsf_file *File);
void HsfFileSeek(hsf_file *File, u32 Offset, int SeekType);
int  HsfFileTell(hsf_file *File);
void HsfFileRead(void *Buffer, size_t size, size_t count, hsf_file *File);

#endif

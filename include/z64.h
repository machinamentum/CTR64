/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef Z64_H
#define Z64_H

#include "platform.h"
#include <cstdio>

#define MANUFACTURER_NINTENDO ('N')

#define COUNTRY_GERMANY       ('D' << 8)
#define COUNTRY_USA           ('E' << 8)
#define COUNTRY_JAPAN         ('J' << 8)
#define COUNTRY_EUROPE        ('P' << 8)
#define COUNTRY_AUSTRALIA     ('U' << 8)

struct __attribute__((packed)) z64_hdr
{
    u8 InitLAT;
    u8 InitPGS0; // 0x37
    u8 InitPWD;
    u8 InitPGS1; // 0x40
    u32 ClockRate;
    u32 PC;
    u32 Release;
    u32 CRC1;
    u32 CRC2;
    u64 Unknown0;
    char ImageName[20];
    u32 Unknown1;
    u32 ManufacturerID;
    u16 CartridgeID;
    u16 Country;
    u32 BootCode[1008];
};

#define Z64_FLAG_MIDDLE_ENDIAN (1 << 0)

struct z64
{
    FILE *FileStream;
    int Flags;
};

void Z64GetHeader(z64 *Z64, z64_hdr *Hdr);
void Z64Read(z64 *Z64, void *buffer, unsigned int bytes);
void Z64Seek(z64 *Z64, unsigned int pos);
int Z64Open(z64 *Z64, int Flags, const char *FileName);
void Z64Close(z64 *Z64);
unsigned int Z64GetCartSize(z64 *Z64);


const char *Z64GetCountryString(u16 Country);
const char *Z64GetManufacturerString(u16 ManufacturerID);

#endif

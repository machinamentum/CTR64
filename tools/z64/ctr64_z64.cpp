/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <cstdio>
#include <cstring>
#include "z64.h"
#include "disasm.h"

void
PrintUsage()
{
    printf("ctr64_z64 <file> [options]\n");
    printf("                 --me    | Decode file in middle-endian mode (V64)\n");
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return 0;
    }

    FILE *File = fopen(argv[1], "rb");
    if (!File)
    {
        printf("Could not open file: %s\n", argv[1]);
        return -1;
    }

    int Flags = 0;

    if (argc > 2)
    {
        if(strncmp(argv[2], "--me", 4) == 0)
        {
            Flags |= Z64_FLAG_MIDDLE_ENDIAN;
        }
    }

    z64 Z64;
    Z64Open(&Z64, Flags, argv[1]);
    printf("Image name:   %.20s\n", Z64.Hdr.ImageName);
    printf("Manufacturer: %s\n", Z64GetManufacturerString(Z64.Hdr.ManufacturerID));
    printf("Region:       %s\n", Z64GetCountryString(Z64.Hdr.Country));
    printf("Boot code:\n");
    MIPS_R3000 Dummy;
    memcpy(Dummy.RAM, &Z64.Hdr.BootCode, 1008 * 4);
    DisassemblerPrintRange(&Dummy, 0, 32, 0);
    Z64Close(&Z64);
    return 0;
}

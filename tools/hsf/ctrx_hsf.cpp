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
#include "hsf.h"

void
PrintUsage()
{
    printf("ctrx_hsf <hsf file> [options]\n");
    printf("         -e <file name> <outfile>   | extract file from hsf file\n");
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return -1;
    }

    hsf HSF;
    HsfOpen(&HSF, argv[1]);
    hsf_primary_volume_descriptor *PVD = HSF.PVD;
    printf("System ID: %.32s\n", PVD->SystemIdentifier);
    printf("Volume ID: %.32s\n", PVD->VolumeIdentifier);

    if (argc > 2)
    {
        for (int i = 2; i < argc; ++i)
        {
            if (strncmp(argv[i], "-e", 2) == 0)
            {
                if (i + 2 >= argc)
                {
                    printf("Not enough args for option -e\n");
                    goto _Exit;
                }
                ++i;

                printf("Extracting file: %s\n", argv[1]);

                hsf_file *File = HsfFileOpen(&HSF, argv[i]);
                HsfFileSeek(File, 0, SEEK_END);
                int Size = HsfFileTell(File);
                printf("File size: %d bytes\n", Size);
                HsfFileSeek(File, 0, SEEK_SET);
                void *Buffer = linearAlloc(Size);
                HsfFileRead(Buffer, Size, 1, File);
                HsfFileClose(File);
                ++i;

                FILE *Out = fopen(argv[i], "wb");
                fwrite(Buffer, Size, 1, Out);
                fclose(Out);
            }
        }
    }

_Exit:
    HsfClose(&HSF);

    return 0;
}

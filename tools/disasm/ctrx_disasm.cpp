/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <string>
#include <cstring>
#include "disasm.h"
#include "psxexe.h"

void
PrintUsage()
{
    printf("ctrx_disasm <file> (hex address) (number of instructions)\n");
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return -1;
    }

    std::string FilePath = std::string(argv[1], 0, strlen(argv[1]));

    MIPS_R3000 DummyCpu;

    FILE *f = fopen(FilePath.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *ExeBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(ExeBuffer, fsize, 1, f);
    fclose(f);

    MapMemoryRegion(&DummyCpu, (mmm) {ExeBuffer, 0, (u32)fsize});

    u32 Address = DummyCpu.pc;
    if (argc > 2)
    {
        std::string AddrStr = std::string(argv[2], 0, strlen(argv[2]));
        Address = std::stoul(AddrStr, nullptr, 16);
    }
    u32 NumInstructions = 32;
    if (argc > 3)
    {
        std::string NumStr = std::string(argv[3], 0, strlen(argv[3]));
        NumInstructions = std::stoul(NumStr, nullptr, 10);
    }

    DisassemblerPrintRange(&DummyCpu, Address, NumInstructions, 0);

    return 0;
}
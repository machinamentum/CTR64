/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "asm.h"


void
PrintUsage()
{
    printf("ctr64_asm <file> <output> [options]\n");
    printf("                 --be    | Encode output in big-endian mode\n");
    printf("                 --le    | Encode output in little-endian mode\n");
}

std::vector<std::string> &
split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string>
split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

struct {const char *Name; u32 Select0;} Select0Table[0x2F] =
{
    {"j", 0x02},
    {"jal", 0x03},
    {"beq", 0x04},
    {"bne", 0x05},
    {"blez", 0x06},
    {"bgtz", 0x07},
    {"addi", 0x08},
    {"addiu", 0x09},
    {"slti", 0x0A},
    {"sltiu", 0x0B},
    {"andi", 0x0C},
    {"ori", 0x0D},
    {"xori", 0x0E},
    {"lui", 0x0F},
//    {"cop0", 0x10},
//    {"cop1", 0x11},
//    {"cop2", 0x12},
//    {"cop3", 0x13},
    {"beql", 0x14},
    {"bnel", 0x15},
    {"blezl", 0x16},
    {"bgtzl", 0x17},
    {"daddi", 0x18},
    {"daddiu", 0x19},
    {"ldl", 0x1A},
    {"ldr", 0x1B},
    {"lb", 0x20},
    {"lh", 0x21},
    {"lwl", 0x22},
    {"lw", 0x23},
    {"lbu", 0x24},
    {"lhu", 0x25},
    {"lwr", 0x26},
    {"lwu", 0x27},
    {"sb", 0x28},
    {"sh", 0x29},
    {"swl", 0x2A},
    {"sw", 0x2B},
    {"sdl", 0x2C},
    {"sdr", 0x2D},
    {"swr", 0x2E},
    {"lwc0", 0x30},
    {"lwc1", 0x31},
    {"lwc2", 0x32},
    {"lwc3", 0x33},
    {"ld", 0x37},
    {"swc0", 0x38},
    {"swc1", 0x39},
    {"swc2", 0x3A},
    {"swc3", 0x3B},
    {"sd", 0x3F},
};

u32
GetSelect0ForInstruction(std::string &str)
{
    for (u32 i = 0; i < 0x2F; ++i)
    {
        if (str.compare(Select0Table[i].Name) == 0)
        {
            return Select0Table[i].Select0;
        }
    }

    return 0;
}

std::vector<u32>
ParseAsmSource(std::string &Source)
{
    std::istringstream SS;
    SS.str(Source);
    std::string Line;
    std::vector<u32> DataArray;
    while (std::getline(SS, Line))
    {
        if (Line.compare("") == 0) continue;
        while (Line.find_first_of(',') != std::string::npos)
        {
            Line.replace(Line.find_first_of(','), 1, " ");
        }
        while (Line.find("  ") != std::string::npos)
        {
            Line.replace(Line.find("  "), 2, " ");
        }
        if (Line.find_first_of(' ') == 0) Line.replace(Line.find_first_of(' '), 1, "");
        std::vector<std::string> Tokens = split(Line, ' ');
        if (Tokens.size() < 2) continue;
        disasm_opcode_info Info = {0};
        Info.Select0 = GetSelect0ForInstruction(Tokens.at(0));

        u32 MachineCode = 0;
        AssemblerTranslateOpCode(&Info, &MachineCode);
        DataArray.push_back(MachineCode);
    }

    return DataArray;
}

int
main(int argc, char **argv)
{
    if (argc < 3)
    {
        PrintUsage();
        return 0;
    }

    std::ifstream InputStream;
    InputStream.open(argv[1], std::ifstream::binary);
    if (InputStream.fail())
    {
        printf("Could not open file: %s\n", argv[1]);
        InputStream.close();
        return -1;
    }

    std::string SourceStr;

    InputStream.seekg(0, std::ios::end);
    SourceStr.reserve(InputStream.tellg());
    InputStream.seekg(0, std::ios::beg);

    SourceStr.assign((std::istreambuf_iterator<char>(InputStream)),
               std::istreambuf_iterator<char>());
    InputStream.close();

    std::vector<u32> DataArray = ParseAsmSource(SourceStr);
    std::ofstream OutputStream;
    OutputStream.open(argv[2], std::ofstream::binary);
    OutputStream.write((char *)&DataArray[0], DataArray.size() * sizeof(u32));
    OutputStream.close();

    return 0;
}
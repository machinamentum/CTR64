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
#include "lexer.h"

void
PrintUsage()
{
    printf("ctr64_asm <file> <output> [options]\n");
    printf("                 --be    | Encode output in big-endian mode\n");
    printf("                 --le    | Encode output in little-endian mode\n");
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

struct {const char *Name; u32 Select1;} Select1Table[0x25] =
{
    {"sll", 0x00},
    {"srl", 0x02},
    {"sra", 0x03},
    {"sllv", 0x04},
    {"srav", 0x06},
    {"jr", 0x08},
    {"jalr", 0x09},
    {"syscall", 0x0C},
    {"break", 0x0D},
    {"mfhi", 0x10},
    {"mthi", 0x11},
    {"mflo", 0x12},
    {"mtlo", 0x13},
    {"mult", 0x18},
    {"multu", 0x19},
    {"div", 0x1A},
    {"divu", 0x1B},
    {"add", 0x20},
    {"addu", 0x21},
    {"sub", 0x22},
    {"subu", 0x23},
    {"and", 0x24},
    {"or", 0x25},
    {"xor", 0x26},
    {"nor", 0x27},
    {"slt", 0x2A},
    {"sltu", 0x2B},
    {"dadd", 0x2C},
    {"daddu", 0x2D},
    {"dsub", 0x2E},
    {"dsubu", 0x2F},
    {"tge", 0x30},
    {"tgeu", 0x31},
    {"tlt", 0x32},
    {"tltu", 0x33},
    {"teq", 0x34},
    {"tne", 0x36},
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

u32
GetSelect1ForInstruction(std::string &str)
{
    for (u32 i = 0; i < 0x25; ++i)
    {
        if (str.compare(Select1Table[i].Name) == 0)
        {
            return Select1Table[i].Select1;
        }
    }

    return -1;
}

enum
{
    FORM_BRANCH_ZERO = 0,
    FORM_JUMP_IMM,
    FORM_BRANCH_REG_COMP,
    FORM_ALU_IMM,
    FORM_LUI,
    FORM_COPROCESSOR,
    FORM_STORE_LOAD,
    FORM_SHIFT_IMM,
    FORM_ALU_REG,
    FORM_JR,
    FORM_JALR,
    FORM_SYSCALL_BREAK,
    FORM_MFHILO,
    FORM_MTHILO,
    FORM_MULT_DIV,
    FORM_TRAP_REG
};

u32
ParserGetInstructionForm(u32 Select0, u32 Select1)
{
    if (Select0)
    {
        if (Select0 == 0x01) return FORM_BRANCH_ZERO;
        if (Select0 >= 0x02 && Select0 <= 0x03) return FORM_JUMP_IMM;
        if (Select0 >= 0x04 && Select0 <= 0x07) return FORM_BRANCH_REG_COMP;
        if (Select0 >= 0x08 && Select0 <= 0x0E) return FORM_ALU_IMM;
        if (Select0 == 0x0F) return FORM_LUI;
        if (Select0 >= 0x10 && Select0 <= 0x13) return FORM_COPROCESSOR;
        if (Select0 >= 0x14 && Select0 <= 0x17) return FORM_BRANCH_REG_COMP;
        if (Select0 >= 0x18 && Select0 <= 0x19) return FORM_ALU_IMM;
        if (Select0 >= 0x1A && Select0 <= 0x3F) return FORM_STORE_LOAD;
    }
    else
    {
        if (Select1 <= 0x03) return FORM_SHIFT_IMM;
        if (Select1 >= 0x04 && Select1 <= 0x07) return FORM_ALU_REG;
        if (Select1 == 0x08) return FORM_JR;
        if (Select1 == 0x09) return FORM_JALR;
        if (Select1 >= 0x0C && Select1 <= 0x0D) return FORM_SYSCALL_BREAK;
        if (Select1 == 0x10) return FORM_MFHILO;
        if (Select1 == 0x11) return FORM_MTHILO;
        if (Select1 == 0x12) return FORM_MFHILO;
        if (Select1 == 0x13) return FORM_MTHILO;
        if (Select1 >= 0x18 && Select1 <= 0x1B) return FORM_MULT_DIV;
        if (Select1 >= 0x20 && Select1 <= 0x2F) return FORM_ALU_REG;
        if (Select1 >= 0x30 && Select1 <= 0x36) return FORM_TRAP_REG;
    }

    return 0;
}

static const char *RNT[34] =
{
    "zr",
    "at",

    "v0",
    "v1",

    "a0",
    "a1",
    "a2",
    "a3",

    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",

    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",

    "k0",
    "k1",

    "gp",
    "sp",
    "fp",

    "ra",
    "pc",
    
    "hi:lo"
};

u32
GetGPRIndexFromString(std::string Str)
{
    for (u32 i = 0; i < 34; ++i)
    {
        if (Str.compare(RNT[i]) == 0) return i;
    }

    if (Str[0] == 'r')
    {
        return strtoul(&Str[1], nullptr, 10);
    }
    printf("error: couldn't map register name '%s' to an index.\n", Str.c_str());
    return -1;
}

void
ParseShiftImmForm(disasm_opcode_info *Info, LexerInstance* Lex, LexerToken &Token)
{
    std::string OpName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 1.\n", OpName.c_str());
        return;
    }
    Info->DestinationRegister = GetGPRIndexFromString(Token.String);
    std::string DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 2.\n", OpName.c_str());
        return;
    }
    Info->RightValue = GetGPRIndexFromString(Token.String);
    DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_INT)
    {
        printf("error: expected integer token for %s paramter 3.\n", OpName.c_str());
        return;
    }
    Info->Immediate = Token.Int;
}

void
ParseStoreLoadForm(disasm_opcode_info *Info, LexerInstance* Lex, LexerToken &Token)
{
    std::string OpName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 1.\n", OpName.c_str());
        return;
    }
    Info->RightValue = GetGPRIndexFromString(Token.String);
    std::string DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_INT)
    {
        printf("error: expected integer token for %s paramter 2.\n", OpName.c_str());
        return;
    }
    Info->Immediate = Token.Int;
    Token = Lex->GetToken();
    if (Token.Type != '(')
    {
        printf("error: expected token '(' after integer token '%ld'.\n", Info->Immediate);
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 3.\n", OpName.c_str());
        return;
    }
    Info->LeftValue = GetGPRIndexFromString(Token.String);
    DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ')')
    {
        printf("Token: %d\n", Token.Type);
        printf("error: expected token ')' after identifier '%s'.\n", DestName.c_str());
        return;
    }
}

void
ParseLUIForm(disasm_opcode_info *Info, LexerInstance* Lex, LexerToken &Token)
{
    std::string OpName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 1.\n", OpName.c_str());
        return;
    }
    Info->DestinationRegister = GetGPRIndexFromString(Token.String);
    std::string DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_INT)
    {
        printf("error: expected integer token for %s paramter.\n", OpName.c_str());
        return;
    }
    Info->Immediate = Token.Int;
}

void
ParseJumpImmForm(disasm_opcode_info *Info, LexerInstance* Lex, LexerToken &Token)
{
    std::string OpName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_INT)
    {
        printf("error: expected integer token for %s paramter.\n", OpName.c_str());
        return;
    }
    Info->Immediate = Token.Int / 4;
}

void
ParseALUImmForm(disasm_opcode_info *Info, LexerInstance* Lex, LexerToken &Token)
{
    std::string OpName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 1.\n", OpName.c_str());
        return;
    }
    Info->DestinationRegister = GetGPRIndexFromString(Token.String);
    std::string DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 2.\n", OpName.c_str());
        return;
    }
    Info->LeftValue = GetGPRIndexFromString(Token.String);
    DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_INT)
    {
        printf("error: expected 16-bit integer for %s paramter 3.\n", OpName.c_str());
        return;
    }
    Info->Immediate = Token.Int;
}

void
ParseMultDivForm(disasm_opcode_info *Info, LexerInstance* Lex, LexerToken &Token)
{
    std::string OpName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 1.\n", OpName.c_str());
        return;
    }
    Info->LeftValue = GetGPRIndexFromString(Token.String);
    std::string DestName = Token.String;
    Token = Lex->GetToken();
    if (Token.Type != ',')
    {
        printf("error: expected token ',' after identifier '%s'.\n", DestName.c_str());
        return;
    }
    Token = Lex->GetToken();
    if (Token.Type != LexerToken::TOKEN_ID)
    {
        printf("error: expected register name for %s paramter 2.\n", OpName.c_str());
        return;
    }
    Info->RightValue = GetGPRIndexFromString(Token.String);
}

u32
ParseInstuctionLine(LexerInstance *Lex, LexerToken &Token)
{
    u32 Data = 0;
    disasm_opcode_info Info;
    Info.Select0 = GetSelect0ForInstruction(Token.String);
    if (Info.Select0 == 0)
    {
        Info.Select1 = GetSelect1ForInstruction(Token.String);
        if (Info.Select1 == -1)
        {
            printf("Unknown instruction: %s\n", Token.String.c_str());
            return ASM_UNK_OP;
        }
    }

    u32 Form = ParserGetInstructionForm(Info.Select0, Info.Select1);
    switch (Form)
    {
        case FORM_JUMP_IMM: ParseJumpImmForm(&Info, Lex, Token); break;
        case FORM_ALU_IMM: ParseALUImmForm(&Info, Lex, Token); break;
        case FORM_LUI: ParseLUIForm(&Info, Lex, Token); break;
        case FORM_STORE_LOAD: ParseStoreLoadForm(&Info, Lex, Token); break;
        case FORM_SHIFT_IMM: ParseShiftImmForm(&Info, Lex, Token); break;
        case FORM_MULT_DIV: ParseMultDivForm(&Info, Lex, Token); break;
    }
    AssemblerTranslateOpCode(&Info, &Data);
    return Data;
}

std::vector<u32>
ParseAsmSource(std::string &Source)
{
    LexerInstance Lex(Source);
    std::vector<u32> DataArray;
    LexerToken Token = Lex.GetToken();
    while (Token.Type != LexerToken::TOKEN_EOF)
    {
        if (Token.Type == LexerToken::TOKEN_ID)
        {
            u32 Data = ParseInstuctionLine(&Lex, Token);
            DataArray.push_back(Data);
        }
        Token = Lex.GetToken();
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
    if (argc > 3 && std::string("--be").compare(argv[3]) == 0)
    {
        for (u32 i = 0; i < DataArray.size(); ++i)
        {
            DataArray[i] = __builtin_bswap32(DataArray[i]);
        }
    }
    OutputStream.open(argv[2], std::ofstream::binary);
    OutputStream.write((char *)&DataArray[0], DataArray.size() * sizeof(u32));
    OutputStream.close();

    return 0;
}
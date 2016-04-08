/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include <string>
#include "lexer.h"

static bool
LexerIsWhiteSpace(char c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') ||  (c == '\f');
}

static LexerToken
MakeIDToken(std::string Id)
{
    LexerToken Token;
    Token.Type = LexerToken::TOKEN_ID;
    Token.String = Id;
    return Token;
}

static LexerToken
MakeEOFToken()
{
    LexerToken Token;
    Token.Type = LexerToken::TOKEN_EOF;
    return Token;
}

static LexerToken
MakeIntToken(u32 Value)
{
    LexerToken Token;
    Token.Type = LexerToken::TOKEN_INT;
    Token.Int = Value;
    return Token;
}

static LexerToken
MakeCodepointToken(u32 c)
{
    LexerToken Token;
    Token.Type = c;
    return Token;
}

LexerInstance::LexerInstance(std::string &Str)
{
    SrcStr = Str;
    SrcPtr = &SrcStr[0];
    EndPtr = SrcPtr + SrcStr.length();
}

LexerToken
LexerInstance::GetToken()
{
    if (SrcPtr + 1 >= EndPtr) return MakeEOFToken();
    while (LexerIsWhiteSpace(*SrcPtr) && (SrcPtr + 1 < EndPtr))
    {
        ++SrcPtr;
    }

    if (((*SrcPtr >= 'A') && (*SrcPtr <= 'Z')) || ((*SrcPtr >= 'a') && (*SrcPtr <= 'z')))
    {
        char *StartPtr = SrcPtr;
        while (!LexerIsWhiteSpace(*SrcPtr) && (SrcPtr + 1 < EndPtr) && (((*SrcPtr >= 'A') && (*SrcPtr <= 'Z')) || ((*SrcPtr >= 'a') && (*SrcPtr <= 'z')) || ((*SrcPtr >= '0') && (*SrcPtr <= '9'))))
        {
            ++SrcPtr;
        }
        return MakeIDToken(std::string(StartPtr, SrcPtr - StartPtr));
    }
    if (((*SrcPtr >= '0') && (*SrcPtr <= '9')))
    {
        if (SrcPtr + 1 < EndPtr)
        {
            switch (*(SrcPtr + 1))
            {
                case 'x':
                {
                    char *End;
                    u32 Value = strtoul(SrcPtr, &End, 16);
                    SrcPtr = End;
                    return MakeIntToken(Value);
                }

                case 'b':
                {
                    char *End;
                    u32 Value = strtoul(SrcPtr, &End, 2);
                    SrcPtr = End;
                    return MakeIntToken(Value);
                }
            }

            if (*SrcPtr == '0')
            {
                char *End;
                u32 Value = strtoul(SrcPtr, &End, 8);
                SrcPtr = End;
                return MakeIntToken(Value);
            }
            else
            {
                char *End;
                u32 Value = strtoul(SrcPtr, &End, 10);
                SrcPtr = End;
                return MakeIntToken(Value);
            }
        }
    }

    return MakeCodepointToken(*SrcPtr++);
}

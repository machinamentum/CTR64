/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef LEXER_H
#define LEXER_H

#include "platform.h"

struct LexerToken
{
    enum
    {
        TOKEN_EOF = 256,
        TOKEN_ID,
        TOKEN_INT,
    };

    u32 Type;
    u32 Int;
    std::string String;
};

struct LexerInstance
{
    char *SrcPtr;
    char *EndPtr;
    std::string SrcStr;

    LexerInstance(std::string &Str);
    LexerToken GetToken();
};


#endif

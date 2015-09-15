/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */

void *
memcpy(void *Dst, const void *Src, unsigned int Length)
{
    if (!Dst) return 0;
    if (Length > 0x7FFFFFFF) return 0;
    char *DstB = (char *)Dst;
    char *SrcB = (char *)Src;
    for (int i = 0; i < Length; ++i)
    {
        DstB[i] = SrcB[i];
    }

    return Dst;
}

void *
bcopy(void *Src, const void *Dst, unsigned int Length)
{
    if (!Src) return 0;
    if (Length > 0x7FFFFFFF) return 0;
    char *DstB = (char *)Dst;
    char *SrcB = (char *)Src;
    for (int i = 0; i < Length; ++i)
    {
        DstB[i] = SrcB[i];
    }

    return Src;
}

void *
memmove(void *Dst, const void *Src, unsigned int Length)
{
    if (!Dst) return 0;
    if (Length > 0x7FFFFFFF) return 0;
    char *DstB = (char *)Dst;
    char *SrcB = (char *)Src;
    if (Src < Dst && (Dst >= (Src + Length)))
    {
        for (int i = Length; i >= 0; --i)
        {
            DstB[i] = SrcB[i];
        }
    }
    else
    {
        for (int i = 0; i < Length; ++i)
        {
            DstB[i] = SrcB[i];
        }
    }

    return Dst;
}

void *
memset(const void *Dst, int FillByte, unsigned int Length)
{
    if (Length > 0x7FFFFFFF || Length == 0)
    {
        return 0;
    }
    if (!Dst) return 0;

    unsigned char *Out = (unsigned char *)Dst;
    for (int i = 0; i < Length; ++i)
    {
        Out[i] = (unsigned char)FillByte;
    }

    return (void *)Dst;
}

void *
bzero(const char *Dst, unsigned int Length)
{
    return memset(Dst, 0, Length);
}

int
memcmp(const void *Src1, const void *Src2, unsigned int Length)
{
    if (!Src1 || !Src2) return 0;

    char *S1 = (char *)Src1;
    char *S2 = (char *)Src2;
    for (int i = 0; i < Length; ++i)
    {
        if (S1[i] != S2[i])
        {
            ++i;
            return S1[i] - S2[i];
        }
    }

    return 0;
}

int
bcmp(const char *Src1, const char *Src2, unsigned int Length)
{
    return memcmp(Src1, Src2, Length);
}

void *
memchr(const void *Src, char ScanByte, unsigned int Length)
{
    if (!Src || Length > 0x7FFFFFFF) return 0;
    char *Str = (char *)Src;

    for (int i = 0; i < Length; ++i)
    {
        if (Str[i] == ScanByte)
        {
            return Str + i;
        }
    }

    return 0;
}

void *HeapStartPtr = 0;
void *HeapPtr = 0;
int HeapSize = 0;

void
InitHeap(void *Address, int Size)
{
    HeapStartPtr = HeapPtr = Address;
    HeapSize = Size;
}

void *
malloc(int Size)
{
    if (Size % 4) Size += (4 - (Size % 4));
    if (HeapStartPtr + HeapSize < HeapPtr + Size) return 0;
    void *Temp = HeapPtr;
    HeapPtr += Size;
    return Temp;
}

void
free(void *Ptr)
{
    unsigned int *Buf = (unsigned int *)(((char *)Ptr) - 4);
    Buf[0] |= 1;
}

void *
calloc(int Num, int Size)
{
    int Total = Num * Size;
    return bzero(malloc(Total), Total);
}

void *
realloc(void *Old, int Size)
{
    return 0;
}

static unsigned int RandGenValue = 0;

unsigned int
rand()
{
    RandGenValue = RandGenValue * 0x41C64E6D + 0x3039;
    return (RandGenValue / 0x10000) & 0x7FFF;
}

void
srand(unsigned int Seed)
{
    RandGenValue = Seed;
}

int
abs(int Value)
{
    return (Value < 0 ? -Value : Value);
}

long int
labs(long int Value)
{
    return (Value < 0 ? -Value : Value);
}

unsigned int
todigit(unsigned int c)
{
    c &= 0xFF;
    if (c >= 0x30 && c <0x3A)
    {
        return c - 0x30;
    }
    if (c > 0x60 && c < 0x7B)
    {
        c -= 0x20;
    }
    if (c > 0x40 && c < 0x5B)
    {
        return c - 0x41 + 10;
    }
    if (c >= 0x80)
    {
        return -1;
    }
    return 0x0098967F;
}

unsigned int
strlen(const char *Src)
{
    if (!Src) return 0;
    unsigned int Length = 0;
    while (*(Src + Length) != '\0');
    {
        ++Length;
    }

    return Length;
}

int
strtol(const char *Str, char **EndPtr, int Base)
{
    return 0;
}

unsigned int
strtoul(const char *Str, char **EndPtr, int Base)
{
    return 0;
}

int
atoi(const char *Src)
{
    return 0;
}

long int
atol(const char *Src)
{
    return 0;
}

void *
atob(const char *Src, int *NumDst)
{
    char *SrcEnd;
    int Result = strtol(Src, &SrcEnd, 10);
    *NumDst = Result;
    return SrcEnd;
}


// NOTE PSX does not contain COP1; this would cause exception normally
float
atof(const char *Str)
{
    return 0.0;
}

double
strtod(const char *Src, char **EndPtr)
{
    return 0.0;
}



void *
strcat(const char *Dst, const char *Src)
{
    return 0;
}

void *
strncat(const char *Dst, const char *Src, int MaxLen)
{
    return 0;
}

int
strcmp(const char *Str1, const char *Str2)
{
    return 0;
}

int
strncmp(const char *Str1, const char *Str2, int MaxLen)
{
    return 0;
}

char *
strcpy(char *Dst, const char *Src)
{
    if (!Src || !Dst) return 0;

    while (*Src != 0)
    {
        *Dst = *Src;
        ++Src;
    }
    *Dst = *Src;
    return Dst;
}

char *
strncpy(char *Dst, const char *Src, int MaxLen)
{
    if (!Src || !Dst) return 0;

    while (*Src != 0)
    {
        --MaxLen;
        if (MaxLen <= 0) return Dst;
        *Dst = *Src;
        ++Src;
    }
    *Dst = *Src;
    while (MaxLen)
    {
        ++Dst;
        *Dst = 0;
        --MaxLen;
    }
    return Dst;
}

char *
index(const char *Src, int Char)
{
    return 0;
}

char *
rindex(const char *Src, int Char)
{
    return 0;
}

char *
strchr(const char *Src, int Char)
{
    return index(Src, Char);
}

char *
strrchr(const char *Src, int Char)
{
    return rindex(Src, Char);
}

char *strpbrk(const char *Src, const char *List)
{
    return 0;
}

int
strspn(const char *Src, const char *List)
{
    return 0;
}

int
strcspn(const char *Src, const char *List)
{
    return 0;
}

char *
strtok(char *Str, const char *List)
{
    return 0;
}

char *
strstr(const char *Str, const char *SubStr)
{
    return 0;
}

int
toupper(int c)
{
    c = c & 0xFF;
    if (c >= 'a' && c <= 'z') return c - 0x20;
    return c;
}

int
tolower(int c)
{
    c = c & 0xFF;
    if (c >= 'A' && c <= 'Z') return c + 0x20;
    return c;
}

void
qsort(void *Base, int Num, int Size, int (*Callback)(const void *, const void *))
{

}

void *
lsearch(const void *Key, void *Base, int Num, int Width, int (*Callback)(const void *, const void *))
{
    return 0;
}

void *
bsearch(const void *Key, void *Base, int Num, int Width, int (*Callback)(const void *, const void *))
{
    return 0;
}

#include "kernel.h"

void
exit(int ExitCode)
{
    SystemErrorExit(ExitCode);
}

#include <stdarg.h>

void
printf(const char *fmt, ...)
{

//    int NumArgs = 0;
//    {
//        char *Str = fmt;
//        while (*Str != 0)
//        {
//            if ( (*Str == '%') && (*(Str + 1) != '%') )
//            {
//                ++NumArgs;
//            }
//            ++Str;
//        }
//    }

    va_list vargs;
    va_start(vargs, fmt);

    char *Str = (char *)fmt;
    char *HexStr;

    while (*Str != 0)
    {
        char Char = *Str;
        if (Char != '%')
        {
            std_out_putchar(Char);
        }
        else
        {
            ++Str;
            Char = *Str;
            switch (Char) {
                case '%':
                    std_out_putchar(Char);
                    break;

                case 'x':
                {
                    HexStr = "0123456789abcdef";
                    unsigned int HexInt = va_arg(vargs, unsigned int);
                    for (int i = 7; i >= 0; --i)
                    {
                        std_out_putchar(HexStr[(HexInt >> (i * 4)) & 0xF]);
                    }
                    break;
                }
                case 'X':
                {
                    HexStr = "0123456789ABCDEF";
                    unsigned int HexInt = va_arg(vargs, unsigned int);
                    for (int i = 7; i >= 0; --i)
                    {
                        std_out_putchar(HexStr[(HexInt >> (i * 4)) & 0xF]);
                    }
                    break;
                }
                case 's':
                {
                    char *PStr = va_arg(vargs, char *);
                    std_out_puts(PStr);
                }
            }
        }


        ++Str;
    }

    va_end(vargs);

}


/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef KERNEL_H
#define KERNEL_H

void *memcpy(void *Dst, const void *Src, unsigned int Length);

void *memmove(void *Dst, const void *Src, unsigned int Length);

void *memset(const void *Dst, int FillByte, unsigned int Length);

int memcmp(const void *Src1, const void *Src2, unsigned int Length);

void *memchr(const void *Src, char ScanByte, unsigned int Length);

void *malloc(int Size);

void free(void *Ptr);

void InitHeap(void *Address, int Size);

unsigned int rand();
void srand(unsigned int Seed);

int abs(int Value);
long int labs(long int Value);
unsigned int todigit(unsigned int c);

unsigned int strlen(const char *Src);

#endif

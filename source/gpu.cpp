/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#include "gpu.h"
#include "platform.h"

#include <cstdio>

static void
GteExecuteOperation(Coprocessor *Cp, u32 FunctionCode)
{

}

GPU::
GPU()
{
    ExecuteOperation = GteExecuteOperation;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 1.0, 0.0, -1.0, 1.0);

#ifdef _3DS
    glTranslatef(0.5f, 0.5f, 0.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);
#endif

    glScalef(1.0 / 320.0, 1.0/240.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    glGenTextures(1, &TempTex);
    glBindTexture(GL_TEXTURE_2D, TempTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, nullptr);
}

typedef void (*gpu_func)(GPU *, u32);
static gpu_func GP0FuncTable[0xFF];

static void
GpuExecuteCommand(GPU *Gpu, u8 Command, u32 Param)
{
    GP0FuncTable[Command](Gpu, Param);
}

u32
GpuStat(void *Object, u32 Address)
{
    GPU *Gpu = (GPU *)Object;
    u32 Status = Gpu->Status;
    Status |= (1 << 26) | (1 << 27) | (1 << 28); // ready flags
    return Status;
}

static u32 *PacketStore = (u32 *)linearAlloc(4 * 1000);
static u32 Packets = 0;

static void
Gp0Cpu2VRAM(GPU* Gpu, u32 Packet)
{
    u32 Width = (Gpu->Gp0Packets[0] & 0xFFFF) / 2;
    u32 Height = (Gpu->Gp0Packets[0] >> 16) & 0xFF;
    u32 Total = Width * Height;
    if (Packets < Total)
    {
        PacketStore[Packets++] = Packet;
        return;
    }
    u32 X = (Gpu->Gp0Packets[1] & 0xFFFF) / 2;
    u32 Y = (Gpu->Gp0Packets[1] >> 16) & 0xFFFF;
//    printf("CPU2VRAM: writing on line %ld\n", Y);
//    printf("CPU2VRAM: writing to X %ld\n", X * 2);
//    printf("CPU2VRAM: Width %ld; Height %ld\n", Width * 2, Height);
    u32 Base = (X + Y * 512);
//    printf("CPU2VRAM: Base %ld\n", Base * 2);
    u32 *VRAM = Gpu->VRAM;
    for (u32 j = 0; j < Height; j++)
    {
        for (u32 i = 0; i < Width; ++i)
        {
            VRAM[Base + i + j * 512] = PacketStore[i + j * Width];
        }
    }
    Gpu->Gp0Func = NULL;
}

void
GpuGp0(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;

    if (Gpu->Gp0PacketsLeft)
    {
        Gpu->Gp0PacketsLeft--;
        Gpu->Gp0Packets[Gpu->Gp0PacketsLeft] = Value;
        if (Gpu->Gp0PacketsLeft == 0)
        {
            GpuExecuteCommand(Gpu, Gpu->Gp0WaitingCmd >> 24, Gpu->Gp0WaitingCmd & 0x00FFFFFF);
        }
        return;
    }

    if (Gpu->Gp0Func)
    {
        Gpu->Gp0Func(Gpu, Value);
        return;
    }
    
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    switch (Command) {
        case 0x01:
            //clear cache
            break;

        case GP0_COMMAND_FILL_RECT:
            Gpu->Gp0WaitingCmd = Value;
            Gpu->Gp0PacketsLeft = 2;
            break;

        case 0xA0:
            Gpu->Gp0WaitingCmd = Value;
            Gpu->Gp0Func = Gp0Cpu2VRAM;
            Gpu->Gp0PacketsLeft = 2;
            Packets = 0;
            break;

        case 0x28:
            Gpu->Gp0WaitingCmd = Value;
            Gpu->Gp0PacketsLeft = 4;
            break;

        case 0xE1:
            Gpu->DrawMode = Param;
            break;

//        case 0xE2:
        case 0xE3:
            Gpu->DrawAreaBounds.X1 = Param & 0b1111111111;
            Gpu->DrawAreaBounds.Y1 = (Param >> 10) & 0b1111111111;
            break;

        case 0xE4:
            Gpu->DrawAreaBounds.X2 = Param & 0b1111111111;
            Gpu->DrawAreaBounds.Y2 = (Param >> 10) & 0b1111111111;
            break;

//        case 0xE5:
//        case 0xE6:
            break;

        case 0x64:
            Gpu->Gp0WaitingCmd = Value;
            Gpu->Gp0PacketsLeft = 3;
            break;

        default:
            printf("GP0 0x%08lX\n", Value);
            break;
    }
}

void
GpuGp1(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    switch (Command)
    {
        case GP1_COMMAND_RST:
            // TODO reset command
            break;

        case 0x01:
            break;

        case GP1_COMMAND_DISP_EN:
            Gpu->Status |= GPU_STAT_DISP_EN;
            break;
        case GP1_COMMAND_DMA_DIR:
            Gpu->Status ^= (-(Param & 3) ^ Gpu->Status) & (3 << 29);
            break;
        default:
            printf("GP1 0x%08lX\n", Value);
            break;
    }


}

static void
GP0Nop(GPU *Gpu, u32 Param)
{

}

static void
GP0FillRect(GPU *Gpu, u32 Param)
{
    float R = (float)(Param & 0xFF) / 255.0f;
    float G = (float)((Param >> 8) & 0xFF) / 255.0f;
    float B = (float)((Param >> 16) & 0xFF) / 255.0f;
    u32 X = Gpu->Gp0Packets[1] & 0xFFFF;
    u32 Y = Gpu->Gp0Packets[1] >> 16;
    u32 W = Gpu->Gp0Packets[0] & 0xFFFF;
    u32 H = Gpu->Gp0Packets[0] >> 16;
    glColor3f(R, G, B);
    glBegin(GL_QUADS);
    glVertex2i(X, Y);
    glVertex2i(X + W, Y);
    glVertex2i(X + W, Y + H);
    glVertex2i(X, Y + H);
    glEnd();
    glColor4f(1, 1, 1, 1);
}

static void
GP0MonochromeOpaqueQuad(GPU* Gpu, u32 Param)
{
    float R = (float)(Param & 0xFF) / 255.0f;
    float G = (float)((Param >> 8) & 0xFF) / 255.0f;
    float B = (float)((Param >> 16) & 0xFF) / 255.0f;
    glColor3f(R, G, B);
    glBegin(GL_QUADS);
    glVertex2i((s32)(Gpu->Gp0Packets[0] & 0xFFFF), (s32)(Gpu->Gp0Packets[0] >> 16));
    glVertex2i((s32)(Gpu->Gp0Packets[1] & 0xFFFF), (s32)(Gpu->Gp0Packets[1] >> 16));
    glVertex2i((s32)(Gpu->Gp0Packets[3] & 0xFFFF), (s32)(Gpu->Gp0Packets[3] >> 16));
    glVertex2i((s32)(Gpu->Gp0Packets[2] & 0xFFFF), (s32)(Gpu->Gp0Packets[2] >> 16));
    glEnd();
    glColor4f(1, 1, 1, 1);
}

inline u16
swap16(u16 in)
{
    return (in << 8) | ((in >> 8) & 0xFF);
}

static void
GP0TexturedRect(GPU *Gpu, u32 Param)
{
//    float R = (float)(Param & 0xFF) / 255.0f;
//    float G = (float)((Param >> 8) & 0xFF) / 255.0f;
//    float B = (float)((Param >> 16) & 0xFF) / 255.0f;
    s32 X = Gpu->Gp0Packets[2] & 0xFFFF;
    s32 Y = (Gpu->Gp0Packets[2] >> 16) & 0xFFFF;
    u32 Width = Gpu->Gp0Packets[0] & 0xFFFF;
    u32 Height = (Gpu->Gp0Packets[0] >> 16) & 0xFFFF;
//    printf("Rect: input 0x%08lX\n", Gpu->Gp0Packets[2]);
//    printf("Rect: X %ld, Y %ld, W %ld, H %ld\n", X, Y, Width, Height);
    // TODO do tex page + palette

    u32 CLUT = (Gpu->Gp0Packets[1] >> 16) & 0xFFFF;
    u32 HalfWordsXCLUT = (CLUT & 0b111111) * 16;
    u32 YCLUT = (CLUT >> 6) & 0b11111111;

    u32 TexIndexX = (Gpu->Gp0Packets[1] & 0xFF);
    u32 TexIndexY = (Gpu->Gp0Packets[1] >> 8) & 0xFF;

//    printf("Rect: Tex Index X %ld\n", TexIndexX);
//    printf("Rect: Tex Index Y %ld\n", TexIndexY);
//
//    printf("Rect: CLUT on line %ld\n", YCLUT);
//    printf("Rect: CLUT X %ld\n", HalfWordsXCLUT);
//    printf("Rect: CLUT Base %ld\n", HalfWordsXCLUT + YCLUT * GPU_VRAM_LINE_SIZE);

    u16 *CLUTStart = ((u16 *)Gpu->VRAM) + HalfWordsXCLUT + YCLUT * GPU_VRAM_LINE_SIZE;

    u32 TexPageX = (Gpu->DrawMode & 0b1111) * 64;
    u32 TexPageY = ((Gpu->DrawMode >> 4) & 1) * 256;
//    printf("Rect: Tex Page X %ld\n", TexPageX);
//    printf("Rect: Tex Page Y %ld\n", TexPageY);
//
//    printf("Rect: Tex Page Base %ld\n", TexPageX + TexPageY * 1024);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Gpu->TempTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    u16 *Buffer = (u16 *)linearAlloc(256 * 256 * 2);
    u32 Base = (TexPageX + TexPageY * 1024) * 2;
    u8 *VRAM = (u8 *)Gpu->VRAM;
    for (u32 j = 0; j < 256; j++)
    {
        for (u32 i = 0; i < 128; ++i)
        {
            Buffer[(i * 2) + j * 256] = swap16(CLUTStart[(VRAM[(Base + i + j * 1024 * 2)] & 0b1111)]);
            Buffer[(i * 2) + 1 + j * 256] = swap16(CLUTStart[ ((VRAM[(Base + i + j * 1024 * 2)] >> 4) & 0b1111) ]);
        }
    }

//    printf("CLUT: ");
//    for (int i = 0; i < 16; ++i)
//    {
//        printf("0x%04X ", CLUTStart[i]);
//    }
//    printf("\n");

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, Buffer);
    linearFree(Buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float TX = (float)TexIndexX / 256.0f;
    float TY = (float)TexIndexY / 256.0f;
    float TW = (float)Width / 256.0f;
    float TH = (float)Height / 256.0f;

    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);

    glTexCoord2f(TX, TY);
    glVertex2i(X, Y);

    glTexCoord2f(TX + TW, TY);
    glVertex2i(X + Width, Y);

    glTexCoord2f(TX + TW, TY + TH);
    glVertex2i(X + Width, Y + Height);

    glTexCoord2f(TX, TY + TH);
    glVertex2i(X, Y + Height);

    glEnd();
    glColor4f(1, 1, 1, 1);
    glDisable(GL_TEXTURE_2D);
}

void
DMA2Trigger(void *Object, u32 Value)
{
    MIPS_R3000 *Cpu = (MIPS_R3000 *)Object;
    DMA *Dma = (DMA *)MapVirtualAddress(Cpu, 0x1F8010A0);
    Dma->CHCR = Value;
//    printf("DMA2\n");
//    printf("Drawing list at 0x%08lX\n", Dma->MADR);
    u32 *List = (u32 *)MapVirtualAddress(Cpu, Dma->MADR);
next_entry:
    u32 EntryHeader = List[0];
    u32 NumCommands = EntryHeader >> 24;
    u32 NextAddr = EntryHeader & 0xFFFFFF;
    for (u32 i = 1; i <= NumCommands; ++i)
    {
        GpuGp0(Cpu->CP2, List[i]);
    }
    if (NextAddr != 0xFFFFFF)
    {
//        printf("Drawing list at 0x%08lX\n", NextAddr);
        List = (u32 *)MapVirtualAddress(Cpu, NextAddr);
        goto next_entry;
    }

//    printf("End DMA2\n\n");
}

static void __attribute__((constructor))
InitFuncTables()
{
    for (int i = 0; i < 0xFF; ++i)
    {
        GP0FuncTable[i] = GP0Nop;
    }

    GP0FuncTable[GP0_COMMAND_FILL_RECT] = GP0FillRect;
    GP0FuncTable[0x28] = GP0MonochromeOpaqueQuad;
    GP0FuncTable[0x64] = GP0TexturedRect;
}

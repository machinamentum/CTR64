
#include <stdint.h>

#define GPCOM(func, param) \
(((func & 0xFF) << 24) | ((param) & 0x00FFFFFF))

uint32_t * const GP0 = (uint32_t *)0x1F801810;
uint32_t * const GP1 = (uint32_t *)0x1F801814;
static const uint32_t *GPUREAD = (uint32_t *)0x1F801810;
static const uint32_t *GPUSTAT = (uint32_t *)0x1F801814;

static inline void write_gp0(uint32_t command)
{
    *GP0 = command;
}

static inline void write_gp1(uint32_t command)
{
    *GP1 = command;
}

static inline void enable_display()
{
    // Reset GPU
    write_gp1(0);

    // turn on display
    write_gp1(GPCOM(0x03, 0));

    // fill rect black
    write_gp0(GPCOM(0x02, 0x000000));
    write_gp0(0); // xy
    write_gp0((240 << 16) | (320));//width, height

    //enable drawing to display
    write_gp0(GPCOM(0xE1, 1 << 10));

    //set drawing area
    write_gp0(GPCOM(0xE3, 0));
    write_gp0(GPCOM(0xE4, 240 | (320 << 10)));
}

const static uint16_t Quad[8] = {
    200, 200,
    0, 200,
    200, 0,
    0, 0
};

static void DrawQuad(const uint16_t *Data)
{
    write_gp0(GPCOM(0x28, 0xFFFFFF));
    write_gp0(Data[0] | (Data[1] << 16));
    write_gp0(Data[2] | (Data[3] << 16));
    write_gp0(Data[4] | (Data[5] << 16));
    write_gp0(Data[6] | (Data[7] << 16));
}

void kmain(void)
{
    enable_display();
    DrawQuad(Quad);
    while (1){}
}

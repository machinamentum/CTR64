
#include <stdint.h>

#define GPCOM(func, param) \
(((func & 0xFF) << 24) | (param & 0x00FFFFFF))

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

    // fill rect red
    write_gp0(GPCOM(0x02, 0x0000ff));
    write_gp0(0); // xy
    write_gp0((240 << 16) | (320));//width, height
}

void kmain(void)
{
    enable_display();
    while (1){}
}
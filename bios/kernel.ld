/* kernel.ld - linker script for free, as in freedom, psx bios */
ENTRY(_start)


SECTIONS
{
    . = 0xBFC00000;
    _start = .;

    .text.boot :
    {
        *(.text.boot)
    }

    . = 0xBFC00100;
    .rodata.rom_header :
    {
        *(.rodata.rom_header)
    }
    . = ALIGN(256);

    .text :
    {
        *(.text)
    }
    . = ALIGN(256);

    .bss :
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(256);

    .data :
    {
        *(.data)
    }

    /DISCARD/ :
    {
        *(.*)
    }

}
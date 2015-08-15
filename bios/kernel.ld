/* kernel.ld - linker script for free, as in freedom, psx bios */
ENTRY(_start)


SECTIONS
{
    . = 0xBFC00000;
    _start = .;

    .text :
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    . = ALIGN(256);

    .rodata :
    {
        *(.rodata)
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

}
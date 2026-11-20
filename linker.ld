ENTRY(_start)
SECTIONS
{
    . = 1M;

    kernel_start = .;

    .text :
    {
        *(.multiboot)
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }

    kernel_end = .;
}
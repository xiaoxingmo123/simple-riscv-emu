MEMORY {
    RAM (rwx) : ORIGIN = 0x0000000000, LENGTH = 8M
}

SECTIONS {
    .text : {
        *(.text.init)
        *(.text. .text.*)
    } > RAM

    .rodata : {
        *(.rodata .rodata.*)
    } > RAM

    .data : {
        *(.data .data.*)
    } > RAM

    .bss : {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        _bss_end = .;
    } > RAM

    _stack_top = ORIGIN(RAM) + LENGTH(RAM);

    _uart_base = 0x10000000;
}
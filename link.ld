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

    STACK_TOP = ORIGIN(RAM) + LENGTH(RAM);
    UART_BASE = 0x10000000;
    MTIME_ADDR = 0x200BFF8;
    MTIMECMP_ADDR = 0x2004000;
}

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* R-- E- */
    data PT_LOAD FLAGS(6); /* RW- -- */
}

SECTIONS
{
    . = 0x100000;  /* Load at 1MB for multiboot compliance */

    /* Multiboot header must be at the beginning */
    .multiboot2 : ALIGN(8) {
        *(.multiboot2)
    } :text

    .text : ALIGN(4096) {
        *(.text .text.*)
    } :text

    .rodata : ALIGN(4096) {
        *(.rodata .rodata.*)
    } :text

    .data : ALIGN(4096) {
        *(.data .data.*)
    } :data

    .bss : ALIGN(4096) {
        *(.bss .bss.*)
        *(COMMON)
    } :data
}
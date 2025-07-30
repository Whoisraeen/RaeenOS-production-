ENTRY(kernel_main)

SECTIONS
{
    /* For ASLR, the base address would be randomized by the bootloader.
     * For now, we keep a fixed base for simplicity.
     * . = 0x100000; */
    . = 0x100000;
    
    .text ALIGN(4K) : {
        *(.multiboot)
        *(.text)
    }
    
    .rodata ALIGN(4K) : {
        *(.rodata)
    }
    
    .data ALIGN(4K) : {
        *(.data)
    }
    
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
}

#include "kernel/include/types.h"
#include "kernel/include/multiboot.h"
#include "kernel/vga.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/pmm.h"
#include "kernel/paging.h"

void kernel_main(uint64_t multiboot_info_addr) {
    vga_init();
    vga_puts("RaeenOS 64-bit Kernel Initializing...\n");

    gdt_init();
    vga_puts("GDT Initialized.\n");

    idt_init();
    vga_puts("IDT Initialized.\n");

    multiboot_info_t* mbi = (multiboot_info_t*)multiboot_info_addr;
    pmm_init_from_mmap(mbi->mmap_addr, mbi->mmap_length);
    vga_puts("PMM Initialized.\n");

    vmm_init();
    vga_puts("VMM Initialized.\n");

    vga_puts("Kernel initialization complete. Halting.\n");
    for(;;);
}


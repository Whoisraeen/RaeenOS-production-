#include "gdt.h"

// Aligned GDT and GDT pointer
gdt_entry_t gdt_entries[GDT_ENTRIES] __attribute__((aligned(0x1000)));
gdt_ptr_t   gdt_ptr;

// TSS (Task State Segment)
tss_t tss __attribute__((aligned(0x1000)));

// Function to set up a GDT entry
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

// Function to set up a TSS entry
void tss_set_gate(int32_t num, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_system_entry_t* tss_entry = (gdt_system_entry_t*)&gdt_entries[num];
    
    tss_entry->limit_low = limit & 0xFFFF;
    tss_entry->base_low = base & 0xFFFF;
    tss_entry->base_middle = (base >> 16) & 0xFF;
    tss_entry->access = access;
    tss_entry->limit_high_gran = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    tss_entry->base_high = (base >> 24) & 0xFF;
    tss_entry->base_upper = (base >> 32) & 0xFFFFFFFF;
    tss_entry->reserved = 0;
}

// Initialize GDT
void gdt_init() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint64_t)&gdt_entries;

    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0);
    // Kernel Code Segment (64-bit)
    gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_EXEC | GDT_ACCESS_RW, GDT_GRAN_LONG_MODE | GDT_GRAN_4K);
    // Kernel Data Segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RW, GDT_GRAN_4K);
    // User Code Segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_EXEC | GDT_ACCESS_RW | GDT_ACCESS_USER, GDT_GRAN_LONG_MODE | GDT_GRAN_4K);
    // User Data Segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RW | GDT_ACCESS_USER, GDT_GRAN_4K);

    // Initialize TSS
    memset(&tss, 0, sizeof(tss_t));
    tss.rsp0 = 0; // Set kernel stack pointer here
    tss.iopb_offset = sizeof(tss_t);
    
    // TSS Segment
    tss_set_gate(5, (uint64_t)&tss, sizeof(tss_t) - 1, GDT_ACCESS_PRESENT | GDT_ACCESS_TSS, 0);

    // Load the GDT and TSS
    gdt_flush((uint64_t)&gdt_ptr);
    tss_flush();
}

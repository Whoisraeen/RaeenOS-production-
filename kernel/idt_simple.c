#include "idt.h"
#include "string.h"

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

// External ISR stubs from assembly
extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_33(void);

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_mid = (base >> 16) & 0xFFFF;
    idt_entries[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].ist     = 0;
    idt_entries[num].flags   = flags;
    idt_entries[num].reserved = 0;
}

void idt_init() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint64_t)&idt_entries;

    // Initialize all IDT entries to 0
    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

    // Setup some basic exception handlers
    idt_set_gate(0, (uint64_t)isr_stub_0, 0x08, 0x8E);   // Divide by zero
    idt_set_gate(1, (uint64_t)isr_stub_1, 0x08, 0x8E);   // Debug
    
    // Setup keyboard interrupt (IRQ1 = interrupt 33)
    idt_set_gate(33, (uint64_t)isr_stub_33, 0x08, 0x8E);

    idt_load((uint64_t)&idt_ptr);
}
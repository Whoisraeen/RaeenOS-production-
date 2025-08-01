// RaeenOS Interrupt Descriptor Table (IDT) C Implementation

#include "vga.h"
#include "syscalls.h"
#include "idt.h"
#include "pic.h"

// External assembly functions for our ISRs and IDT loading
extern void idt_load(struct idt_ptr_t*);
extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

// IRQ handlers
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

// IDT, IDT pointer, and interrupt handlers array
struct idt_entry_t idt_entries[256];
struct idt_ptr_t   idt_ptr;
isr_t interrupt_handlers[256];

// Function to set a gate in the IDT
static void idt_set_gate(uint8_t num, uintptr_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags /* | 0x60 */;
}

// Main IDT initialization function
void idt_init() {
    idt_ptr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uintptr_t)&idt_entries;

    // Clear the IDT and handlers array by zeroing them out
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
        interrupt_handlers[i] = 0;
    }

    // Set gates for the first 32 ISRs (CPU exceptions)
    idt_set_gate(0, (uintptr_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uintptr_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uintptr_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uintptr_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uintptr_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uintptr_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uintptr_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uintptr_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uintptr_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uintptr_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uintptr_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uintptr_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uintptr_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uintptr_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uintptr_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uintptr_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uintptr_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uintptr_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uintptr_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uintptr_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uintptr_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uintptr_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uintptr_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uintptr_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uintptr_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uintptr_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uintptr_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uintptr_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uintptr_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uintptr_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uintptr_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uintptr_t)isr31, 0x08, 0x8E);

    // IRQs (Hardware Interrupts)
    idt_set_gate(32, (uintptr_t)isr32, 0x08, 0x8E);
    idt_set_gate(33, (uintptr_t)isr33, 0x08, 0x8E);
    idt_set_gate(34, (uintptr_t)isr34, 0x08, 0x8E);
    idt_set_gate(35, (uintptr_t)isr35, 0x08, 0x8E);
    idt_set_gate(36, (uintptr_t)isr36, 0x08, 0x8E);
    idt_set_gate(37, (uintptr_t)isr37, 0x08, 0x8E);
    idt_set_gate(38, (uintptr_t)isr38, 0x08, 0x8E);
    idt_set_gate(39, (uintptr_t)isr39, 0x08, 0x8E);
    idt_set_gate(40, (uintptr_t)isr40, 0x08, 0x8E);
    idt_set_gate(41, (uintptr_t)isr41, 0x08, 0x8E);
    idt_set_gate(42, (uintptr_t)isr42, 0x08, 0x8E);
    idt_set_gate(43, (uintptr_t)isr43, 0x08, 0x8E);
    idt_set_gate(44, (uintptr_t)isr44, 0x08, 0x8E);
    idt_set_gate(45, (uintptr_t)isr45, 0x08, 0x8E);
    idt_set_gate(46, (uintptr_t)isr46, 0x08, 0x8E);
    idt_set_gate(47, (uintptr_t)isr47, 0x08, 0x8E);

    // Load the IDT pointer
    idt_load(&idt_ptr);
}

// Function to register a custom handler for a given interrupt
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// General C-level ISR dispatcher
void isr_handler(struct registers_t* regs) {
    // Check if we have a custom handler to run for this interrupt
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } 
    // Handle hardware interrupts (IRQs) by sending EOI
    else if (regs->int_no >= 32 && regs->int_no < 48) {
        // Default IRQ handler if no specific one is registered
    } 
    // Handle unhandled software interrupts and exceptions
    else {
        // For now, unhandled exceptions will just print a message and halt.
        vga_puts("Unhandled exception: ");
        // Simple int-to-string for now
        char num_buf[4];
        int n = regs->int_no;
        if (n == 0) { vga_puts("0"); } else {
            num_buf[3] = 0;
            int i = 2;
            while (n > 0 && i >= 0) { num_buf[i--] = (n % 10) + '0'; n /= 10; }
            vga_puts(&num_buf[i+1]);
        }
        vga_puts("\n");
        asm volatile ("cli; hlt");
    }

    // If it's a hardware interrupt (IRQ), we must send an End-of-Interrupt (EOI)
    if (regs->int_no >= 32 && regs->int_no < 48) {
        pic_send_eoi(regs->int_no - 32);
    }
}

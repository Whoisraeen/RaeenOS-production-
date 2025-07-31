// RaeenOS Interrupt Descriptor Table (IDT) C Interface

#ifndef IDT_H
#define IDT_H

#include "include/types.h"

// The first 32 interrupts are reserved for CPU exceptions.
// IRQs are remapped to start at interrupt 32.
#define IRQ_TO_INT(irq) (irq + 32)


// An entry in the IDT
struct idt_entry_t {
    uint16_t base_lo;   // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t sel;       // Kernel segment selector.
    uint8_t  always0;   // This must always be zero.
    uint8_t  flags;     // More flags. See documentation.
    uint16_t base_hi;   // The upper 16 bits of the address to jump to.
} __attribute__((packed));

// A pointer to the array of interrupt handlers.
struct idt_ptr_t {
    uint16_t limit;
    uintptr_t base;      // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

// A struct describing a register state.
struct registers_t {
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
};
typedef struct registers_t registers_t;

// Typedef for our interrupt handler function pointer
// Typedef for our interrupt handler function pointer. It takes a pointer to the register state.
typedef void (*isr_t)(registers_t*);

// Internal function declarations (implemented in idt.c)
// idt_set_gate is now static in idt.c - no longer exposed

// Function to register a custom handler for a given interrupt
void register_interrupt_handler(uint8_t n, isr_t handler);

// Main IDT initialization function
void idt_init(void);

#endif

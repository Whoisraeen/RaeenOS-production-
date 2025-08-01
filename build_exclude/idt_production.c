/**
 * @file idt_production.c
 * @brief Production-Grade Interrupt Descriptor Table Implementation
 * 
 * This file implements a comprehensive interrupt handling system for RaeenOS
 * with proper IDT setup, exception handling, and interrupt management.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/types.h"
#include "include/sync.h"
#include "include/errno.h"
#include "idt.h"
#include "vga.h"
#include "string.h"

// IDT entry structure (x86-64)
typedef struct idt_entry {
    uint16_t offset_low;    // Offset bits 0-15
    uint16_t selector;      // Code segment selector
    uint8_t ist;           // Interrupt Stack Table offset
    uint8_t type_attr;     // Type and attributes
    uint16_t offset_mid;   // Offset bits 16-31
    uint32_t offset_high;  // Offset bits 32-63
    uint32_t reserved;     // Reserved
} __attribute__((packed)) idt_entry_t;

// IDT descriptor
typedef struct idt_descriptor {
    uint16_t limit;        // Size of IDT - 1
    uint64_t base;         // Base address of IDT
} __attribute__((packed)) idt_descriptor_t;

// Exception frame structure
typedef struct exception_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t interrupt_number;
    uint64_t error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} exception_frame_t;

// Interrupt handler function type
typedef void (*interrupt_handler_t)(exception_frame_t* frame);

// IDT constants
#define IDT_ENTRIES 256
#define IDT_TYPE_INTERRUPT_GATE 0x8E
#define IDT_TYPE_TRAP_GATE 0x8F
#define IDT_TYPE_TASK_GATE 0x85

// Exception numbers
#define EXCEPTION_DIVIDE_ERROR 0
#define EXCEPTION_DEBUG 1
#define EXCEPTION_NMI 2
#define EXCEPTION_BREAKPOINT 3
#define EXCEPTION_OVERFLOW 4
#define EXCEPTION_BOUND_RANGE 5
#define EXCEPTION_INVALID_OPCODE 6
#define EXCEPTION_DEVICE_NOT_AVAILABLE 7
#define EXCEPTION_DOUBLE_FAULT 8
#define EXCEPTION_INVALID_TSS 10
#define EXCEPTION_SEGMENT_NOT_PRESENT 11
#define EXCEPTION_STACK_FAULT 12
#define EXCEPTION_GENERAL_PROTECTION 13
#define EXCEPTION_PAGE_FAULT 14
#define EXCEPTION_X87_FAULT 16
#define EXCEPTION_ALIGNMENT_CHECK 17
#define EXCEPTION_MACHINE_CHECK 18
#define EXCEPTION_SIMD_FAULT 19

// IRQ base (after exceptions)
#define IRQ_BASE 32

// IDT manager structure
typedef struct idt_manager {
    bool initialized;
    idt_entry_t idt[IDT_ENTRIES];
    idt_descriptor_t idt_desc;
    
    // Interrupt handlers
    interrupt_handler_t handlers[IDT_ENTRIES];
    
    // Statistics
    struct {
        uint64_t exception_counts[32];
        uint64_t irq_counts[224];
        uint64_t total_interrupts;
        uint64_t spurious_interrupts;
    } stats;
    
    // Configuration
    struct {
        bool debug_exceptions;
        bool handle_page_faults;
        bool handle_gpf;
    } config;
    
    spinlock_t lock;
} idt_manager_t;

// Global IDT manager
static idt_manager_t idt_manager;
static idt_manager_t* idt = &idt_manager;

// Exception names for debugging
static const char* exception_names[] = {
    "Divide Error",
    "Debug Exception",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Exception"
};

// Forward declarations
static void idt_set_entry(int index, uint64_t handler, uint16_t selector, uint8_t type);
static void default_exception_handler(exception_frame_t* frame);
static void page_fault_handler(exception_frame_t* frame);
static void general_protection_fault_handler(exception_frame_t* frame);
static void double_fault_handler(exception_frame_t* frame);
static void default_irq_handler(exception_frame_t* frame);

// Assembly interrupt stubs (would be in separate .asm file)
extern void isr0(void);   // Divide Error
extern void isr1(void);   // Debug
extern void isr2(void);   // NMI
extern void isr3(void);   // Breakpoint
extern void isr4(void);   // Overflow
extern void isr5(void);   // Bound Range
extern void isr6(void);   // Invalid Opcode
extern void isr7(void);   // Device Not Available
extern void isr8(void);   // Double Fault
extern void isr10(void);  // Invalid TSS
extern void isr11(void);  // Segment Not Present
extern void isr12(void);  // Stack Fault
extern void isr13(void);  // General Protection
extern void isr14(void);  // Page Fault
extern void isr16(void);  // x87 FPU Error
extern void isr17(void);  // Alignment Check
extern void isr18(void);  // Machine Check
extern void isr19(void);  // SIMD Exception

// IRQ stubs
extern void irq0(void);   // Timer
extern void irq1(void);   // Keyboard
// ... more IRQ stubs

/**
 * Initialize the Interrupt Descriptor Table
 */
int idt_init(void) {
    vga_puts("IDT: Initializing production interrupt descriptor table...\n");
    
    // Clear IDT manager structure
    memset(idt, 0, sizeof(idt_manager_t));
    
    // Initialize lock
    spinlock_init(&idt->lock);
    
    // Set configuration
    idt->config.debug_exceptions = true;
    idt->config.handle_page_faults = true;
    idt->config.handle_gpf = true;
    
    // Clear IDT entries
    memset(idt->idt, 0, sizeof(idt->idt));
    
    // Set up IDT descriptor
    idt->idt_desc.limit = sizeof(idt->idt) - 1;
    idt->idt_desc.base = (uint64_t)&idt->idt;
    
    // Install exception handlers
    idt_set_entry(EXCEPTION_DIVIDE_ERROR, (uint64_t)isr0, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_DEBUG, (uint64_t)isr1, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_NMI, (uint64_t)isr2, 0x08, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(EXCEPTION_BREAKPOINT, (uint64_t)isr3, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_OVERFLOW, (uint64_t)isr4, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_BOUND_RANGE, (uint64_t)isr5, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_INVALID_OPCODE, (uint64_t)isr6, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_DEVICE_NOT_AVAILABLE, (uint64_t)isr7, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_DOUBLE_FAULT, (uint64_t)isr8, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_INVALID_TSS, (uint64_t)isr10, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_SEGMENT_NOT_PRESENT, (uint64_t)isr11, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_STACK_FAULT, (uint64_t)isr12, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_GENERAL_PROTECTION, (uint64_t)isr13, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_PAGE_FAULT, (uint64_t)isr14, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_X87_FAULT, (uint64_t)isr16, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_ALIGNMENT_CHECK, (uint64_t)isr17, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_MACHINE_CHECK, (uint64_t)isr18, 0x08, IDT_TYPE_TRAP_GATE);
    idt_set_entry(EXCEPTION_SIMD_FAULT, (uint64_t)isr19, 0x08, IDT_TYPE_TRAP_GATE);
    
    // Install IRQ handlers
    idt_set_entry(IRQ_BASE + 0, (uint64_t)irq0, 0x08, IDT_TYPE_INTERRUPT_GATE);  // Timer
    idt_set_entry(IRQ_BASE + 1, (uint64_t)irq1, 0x08, IDT_TYPE_INTERRUPT_GATE);  // Keyboard
    
    // Set default handlers
    idt->handlers[EXCEPTION_DIVIDE_ERROR] = default_exception_handler;
    idt->handlers[EXCEPTION_DEBUG] = default_exception_handler;
    idt->handlers[EXCEPTION_NMI] = default_exception_handler;
    idt->handlers[EXCEPTION_BREAKPOINT] = default_exception_handler;
    idt->handlers[EXCEPTION_OVERFLOW] = default_exception_handler;
    idt->handlers[EXCEPTION_BOUND_RANGE] = default_exception_handler;
    idt->handlers[EXCEPTION_INVALID_OPCODE] = default_exception_handler;
    idt->handlers[EXCEPTION_DEVICE_NOT_AVAILABLE] = default_exception_handler;
    idt->handlers[EXCEPTION_DOUBLE_FAULT] = double_fault_handler;
    idt->handlers[EXCEPTION_INVALID_TSS] = default_exception_handler;
    idt->handlers[EXCEPTION_SEGMENT_NOT_PRESENT] = default_exception_handler;
    idt->handlers[EXCEPTION_STACK_FAULT] = default_exception_handler;
    idt->handlers[EXCEPTION_GENERAL_PROTECTION] = general_protection_fault_handler;
    idt->handlers[EXCEPTION_PAGE_FAULT] = page_fault_handler;
    idt->handlers[EXCEPTION_X87_FAULT] = default_exception_handler;
    idt->handlers[EXCEPTION_ALIGNMENT_CHECK] = default_exception_handler;
    idt->handlers[EXCEPTION_MACHINE_CHECK] = default_exception_handler;
    idt->handlers[EXCEPTION_SIMD_FAULT] = default_exception_handler;
    
    // Set default IRQ handlers
    for (int i = IRQ_BASE; i < IDT_ENTRIES; i++) {
        idt->handlers[i] = default_irq_handler;
    }
    
    // Load IDT
    __asm__ volatile("lidt %0" :: "m"(idt->idt_desc));
    
    idt->initialized = true;
    
    vga_puts("IDT: Interrupt descriptor table initialized successfully\n");
    return 0;
}

/**
 * Set an IDT entry
 */
static void idt_set_entry(int index, uint64_t handler, uint16_t selector, uint8_t type) {
    if (index < 0 || index >= IDT_ENTRIES) {
        return;
    }
    
    idt_entry_t* entry = &idt->idt[index];
    
    entry->offset_low = (uint16_t)(handler & 0xFFFF);
    entry->offset_mid = (uint16_t)((handler >> 16) & 0xFFFF);
    entry->offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    entry->selector = selector;
    entry->ist = 0;  // No IST for now
    entry->type_attr = type;
    entry->reserved = 0;
}

/**
 * Register an interrupt handler
 */
int idt_register_handler(int interrupt, interrupt_handler_t handler) {
    if (!idt->initialized || interrupt < 0 || interrupt >= IDT_ENTRIES || !handler) {
        return -EINVAL;
    }
    
    spin_lock(&idt->lock);
    idt->handlers[interrupt] = handler;
    spin_unlock(&idt->lock);
    
    return 0;
}

/**
 * Common interrupt entry point (called from assembly stubs)
 */
void idt_common_handler(exception_frame_t* frame) {
    if (!idt->initialized) {
        return;
    }
    
    uint64_t interrupt_num = frame->interrupt_number;
    
    // Update statistics
    idt->stats.total_interrupts++;
    
    if (interrupt_num < 32) {
        idt->stats.exception_counts[interrupt_num]++;
    } else if (interrupt_num >= IRQ_BASE && interrupt_num < IDT_ENTRIES) {
        idt->stats.irq_counts[interrupt_num - IRQ_BASE]++;
    }
    
    // Call registered handler
    if (interrupt_num < IDT_ENTRIES && idt->handlers[interrupt_num]) {
        idt->handlers[interrupt_num](frame);
    } else {
        idt->stats.spurious_interrupts++;
        vga_puts("IDT: Spurious interrupt: ");
        
        // Simple number printing
        char num_str[32];
        uint64_to_string(interrupt_num, num_str, sizeof(num_str));
        vga_puts(num_str);
        vga_puts("\n");
    }
    
    // Send EOI for IRQs (simplified)
    if (interrupt_num >= IRQ_BASE && interrupt_num < IRQ_BASE + 16) {
        // Send EOI to PIC
        if (interrupt_num >= IRQ_BASE + 8) {
            // Secondary PIC
            __asm__ volatile("outb %0, %1" :: "a"(0x20), "Nd"(0xA0));
        }
        // Primary PIC
        __asm__ volatile("outb %0, %1" :: "a"(0x20), "Nd"(0x20));
    }
}

/**
 * Default exception handler
 */
static void default_exception_handler(exception_frame_t* frame) {
    uint64_t exception_num = frame->interrupt_number;
    
    vga_puts("EXCEPTION: ");
    if (exception_num < sizeof(exception_names) / sizeof(exception_names[0])) {
        vga_puts(exception_names[exception_num]);
    } else {
        vga_puts("Unknown Exception");
    }
    vga_puts("\n");
    
    vga_puts("RIP: 0x");
    char addr_str[32];
    uint64_to_hex_string(frame->rip, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts("\n");
    
    vga_puts("Error Code: 0x");
    uint64_to_hex_string(frame->error_code, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts("\n");
    
    // For now, halt the system
    vga_puts("System halted due to unhandled exception\n");
    __asm__ volatile("cli; hlt");
}

/**
 * Page fault handler
 */
static void page_fault_handler(exception_frame_t* frame) {
    uint64_t fault_addr;
    __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
    
    vga_puts("PAGE FAULT: ");
    vga_puts("Address: 0x");
    char addr_str[32];
    uint64_to_hex_string(fault_addr, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts(" RIP: 0x");
    uint64_to_hex_string(frame->rip, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts("\n");
    
    // Analyze error code
    if (frame->error_code & 1) vga_puts("  Protection violation\n");
    else vga_puts("  Page not present\n");
    
    if (frame->error_code & 2) vga_puts("  Write access\n");
    else vga_puts("  Read access\n");
    
    if (frame->error_code & 4) vga_puts("  User mode\n");
    else vga_puts("  Kernel mode\n");
    
    // In production, this would try to handle the fault
    // For now, halt the system
    vga_puts("System halted due to page fault\n");
    __asm__ volatile("cli; hlt");
}

/**
 * General protection fault handler
 */
static void general_protection_fault_handler(exception_frame_t* frame) {
    vga_puts("GENERAL PROTECTION FAULT\n");
    vga_puts("RIP: 0x");
    char addr_str[32];
    uint64_to_hex_string(frame->rip, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts("\n");
    
    vga_puts("Error Code: 0x");
    uint64_to_hex_string(frame->error_code, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts("\n");
    
    // For now, halt the system
    vga_puts("System halted due to general protection fault\n");
    __asm__ volatile("cli; hlt");
}

/**
 * Double fault handler
 */
static void double_fault_handler(exception_frame_t* frame) {
    vga_puts("DOUBLE FAULT - CRITICAL ERROR\n");
    vga_puts("RIP: 0x");
    char addr_str[32];
    uint64_to_hex_string(frame->rip, addr_str, sizeof(addr_str));
    vga_puts(addr_str);
    vga_puts("\n");
    
    // Double fault is unrecoverable
    vga_puts("System halted due to double fault\n");
    __asm__ volatile("cli; hlt");
}

/**
 * Default IRQ handler
 */
static void default_irq_handler(exception_frame_t* frame) {
    // Default IRQ handler - just acknowledge the interrupt
    uint64_t irq_num = frame->interrupt_number - IRQ_BASE;
    
    vga_puts("IRQ: ");
    char num_str[32];
    uint64_to_string(irq_num, num_str, sizeof(num_str));
    vga_puts(num_str);
    vga_puts("\n");
}

/**
 * Enable interrupts
 */
void idt_enable_interrupts(void) {
    __asm__ volatile("sti");
}

/**
 * Disable interrupts
 */
void idt_disable_interrupts(void) {
    __asm__ volatile("cli");
}

/**
 * Get interrupt statistics
 */
int idt_get_stats(struct idt_stats* stats) {
    if (!stats || !idt->initialized) {
        return -EINVAL;
    }
    
    spin_lock(&idt->lock);
    
    stats->total_interrupts = idt->stats.total_interrupts;
    stats->spurious_interrupts = idt->stats.spurious_interrupts;
    
    memcpy(stats->exception_counts, idt->stats.exception_counts, 
           sizeof(stats->exception_counts));
    memcpy(stats->irq_counts, idt->stats.irq_counts, 
           sizeof(stats->irq_counts));
    
    spin_unlock(&idt->lock);
    
    return 0;
}

/**
 * Hex string conversion for debugging
 */
void uint64_to_hex_string(uint64_t value, char* buffer, size_t buffer_size) {
    if (buffer_size < 17) return;  // Need at least 16 chars + null
    
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
}

/**
 * Simple decimal string conversion (imported from PMM)
 */
void uint64_to_string(uint64_t value, char* buffer, size_t buffer_size) {
    if (buffer_size < 2) return;
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    char temp[32];
    int pos = 0;
    
    while (value > 0 && pos < 31) {
        temp[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Reverse string
    int i = 0;
    while (pos > 0 && i < buffer_size - 1) {
        buffer[i++] = temp[--pos];
    }
    buffer[i] = '\0';
}

/**
 * Cleanup IDT
 */
void idt_cleanup(void) {
    idt->initialized = false;
}
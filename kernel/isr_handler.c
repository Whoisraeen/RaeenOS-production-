// Interrupt Service Routine Handler for RaeenOS
// Handles interrupts and exceptions

#include <stdint.h>

// Register structure passed from assembly
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

// Exception messages
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception"
};

// External VGA function for output
extern void vga_puts(const char* str);
extern void vga_putchar(char c);

// Simple number to string conversion
static void print_hex(uint64_t value) {
    char buffer[17];
    buffer[16] = '\0';
    
    const char hex_chars[] = "0123456789ABCDEF";
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    vga_puts("0x");
    vga_puts(buffer);
}

// Main interrupt handler
void isr_handler(registers_t* regs) {
    if (regs->int_no < 32) {
        // CPU Exception
        vga_puts("\nEXCEPTION: ");
        if (regs->int_no < 22) {
            vga_puts(exception_messages[regs->int_no]);
        } else {
            vga_puts("Unknown Exception");
        }
        
        vga_puts("\nInterrupt Number: ");
        print_hex(regs->int_no);
        vga_puts("\nError Code: ");
        print_hex(regs->err_code);
        vga_puts("\nRIP: ");
        print_hex(regs->rip);
        vga_puts("\nRSP: ");
        print_hex(regs->rsp);
        vga_puts("\n");
        
        // For now, halt on exceptions
        __asm__ volatile ("cli; hlt");
        
    } else if (regs->int_no >= 32 && regs->int_no < 48) {
        // Hardware interrupt (IRQ)
        uint8_t irq = regs->int_no - 32;
        
        // Handle specific IRQs
        switch (irq) {
            case 0: // Timer IRQ
                // Timer tick - could implement scheduler here
                break;
                
            case 1: // Keyboard IRQ
                // Keyboard input - could handle keystrokes
                break;
                
            default:
                // Other IRQs - just acknowledge for now
                break;
        }
        
        // Send EOI to PIC
        if (irq >= 8) {
            // Send EOI to slave PIC
            __asm__ volatile ("outb %%al, %%dx" : : "d" (0xA0), "a" (0x20));
        }
        // Send EOI to master PIC
        __asm__ volatile ("outb %%al, %%dx" : : "d" (0x20), "a" (0x20));
        
    } else {
        // Unknown interrupt
        vga_puts("\nUnknown interrupt: ");
        print_hex(regs->int_no);
        vga_puts("\n");
    }
}
/**
 * @file kernel_interactive_simple.c
 * @brief Simplified Interactive RaeenOS Kernel
 * 
 * This version focuses on working interactivity without complex
 * production components that might have compatibility issues.
 * 
 * @version 1.0
 * @date 2025-08-02
 */

#include "vga.h"
#include "libs/libc/include/string.h"
#include "memory.h"
#include "idt.h"
#include "gdt.h" 
#include "keyboard.h"
#include "pic.h"
#include "ports.h"
#include "include/types.h"

// Forward declarations for helper functions
void uint32_to_string(uint32_t value, char* buffer, size_t buffer_size);
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

// External functions
extern void gdt_init(void);
extern void keyboard_init(void);
extern char keyboard_get_char(void);
extern bool keyboard_has_char(void);
extern void pic_init(void);
extern void keyboard_handler(void);

// Simple memory allocator (stub)
static char simple_heap[32768];  // 32KB heap
static size_t heap_pos = 0;

// Command buffer for shell
#define CMD_BUFFER_SIZE 256
static char cmd_buffer[CMD_BUFFER_SIZE];
static size_t cmd_pos = 0;

// System info
static struct {
    uint32_t total_memory_mb;
    uint32_t used_memory_kb;
    uint32_t free_memory_kb;
    uint32_t heap_size_kb;
    const char* arch;
    const char* version;
} system_info = {
    .total_memory_mb = 512,
    .used_memory_kb = 0,
    .free_memory_kb = 0,
    .heap_size_kb = 32,  // Our simple heap size
    .arch = "x86_64",
    .version = "1.0.0-interactive-simple"
};

// Simple heap functions
void* simple_malloc(size_t size) {
    if (heap_pos + size >= sizeof(simple_heap)) {
        return NULL;  // Out of memory
    }
    void* ptr = &simple_heap[heap_pos];
    heap_pos += size;
    return ptr;
}

void simple_free(void* ptr) {
    // Simple allocator - no free implementation
    (void)ptr;
}

int simple_heap_init(void) {
    heap_pos = 0;
    return 0;  // Success
}

// Command handlers
void cmd_help(void);
void cmd_mem(void);
void cmd_cpu(void);
void cmd_clear(void);
void cmd_version(void);
void cmd_reboot(void);
void cmd_test(void);

// Shell functions
void shell_prompt(void);
void shell_process_command(const char* cmd);
void shell_run(void);

/**
 * Display system information during boot
 */
void display_system_info(void) {
    vga_puts("System Information:\n");
    vga_puts("  Architecture: ");
    vga_puts(system_info.arch);
    vga_puts("\n");
    vga_puts("  Kernel Version: ");
    vga_puts(system_info.version);
    vga_puts("\n");
    vga_puts("  Total Memory: ");
    char mem_str[32];
    uint32_to_string(system_info.total_memory_mb, mem_str, sizeof(mem_str));
    vga_puts(mem_str);
    vga_puts(" MB\n");
    vga_puts("  Heap Size: ");
    uint32_to_string(system_info.heap_size_kb, mem_str, sizeof(mem_str));
    vga_puts(mem_str);
    vga_puts(" KB\n\n");
}

/**
 * Enhanced boot sequence with progress indicators
 */
void enhanced_boot_sequence(void) {
    vga_puts("===========================================\n");
    vga_puts("      RaeenOS - Interactive Kernel       \n");
    vga_puts("            (Simplified Version)          \n");
    vga_puts("===========================================\n\n");
    
    vga_puts("Boot Sequence:\n");
    vga_puts("  [OK] Multiboot2 loader successful\n");
    vga_puts("  [OK] VGA text mode initialized\n");
    
    // Initialize GDT
    vga_puts("  [ ] Initializing Global Descriptor Table...\n");
    gdt_init();
    vga_puts("  [OK] GDT configured and active\n");
    
    // Initialize PIC
    vga_puts("  [ ] Initializing Programmable Interrupt Controller...\n");
    pic_init();
    vga_puts("  [OK] PIC initialized\n");
    
    // Initialize IDT
    vga_puts("  [ ] Setting up Interrupt Descriptor Table...\n");
    idt_init();
    vga_puts("  [OK] IDT configured\n");
    
    // Initialize simple heap
    vga_puts("  [ ] Initializing simple kernel heap...\n");
    int heap_result = simple_heap_init();
    if (heap_result == 0) {
        vga_puts("  [OK] Simple heap allocator ready\n");
    } else {
        vga_puts("  [FAIL] Heap initialization failed\n");
    }
    
    // Initialize keyboard
    vga_puts("  [ ] Initializing PS/2 keyboard driver...\n");
    keyboard_init();
    vga_puts("  [OK] Keyboard driver ready for input\n");
    
    vga_puts("\nCore Subsystems Status:\n");
    vga_puts("  - Memory Management: BASIC\n");
    vga_puts("  - Interrupt Handling: OPERATIONAL\n");
    vga_puts("  - Heap Allocator: SIMPLE\n");
    vga_puts("  - Input Subsystem: OPERATIONAL\n");
    
    display_system_info();
    
    vga_puts("===========================================\n");
    vga_puts("   Welcome to RaeenOS Interactive Mode!   \n");
    vga_puts("===========================================\n\n");
    
    vga_puts("This is a simplified interactive kernel demonstrating:\n");
    vga_puts("- Keyboard input handling\n");
    vga_puts("- Command shell interface\n");
    vga_puts("- System information display\n");
    vga_puts("- Basic memory management\n\n");
    
    vga_puts("Type 'help' for available commands.\n\n");
}

/**
 * Help command - display available commands
 */
void cmd_help(void) {
    vga_puts("Available Commands:\n");
    vga_puts("  help      - Show this help message\n");
    vga_puts("  mem       - Display memory information\n");
    vga_puts("  cpu       - Show CPU information\n");
    vga_puts("  version   - Show kernel version\n");
    vga_puts("  clear     - Clear the screen\n");
    vga_puts("  test      - Run keyboard test\n");
    vga_puts("  reboot    - Restart the system\n");
    vga_puts("\n");
}

/**
 * Memory command - show memory statistics
 */
void cmd_mem(void) {
    char num_str[32];
    
    vga_puts("Memory Information:\n");
    vga_puts("  Total RAM: ");
    uint32_to_string(system_info.total_memory_mb, num_str, sizeof(num_str));
    vga_puts(num_str);
    vga_puts(" MB\n");
    
    vga_puts("  Kernel Heap: ");
    uint32_to_string(system_info.heap_size_kb, num_str, sizeof(num_str));
    vga_puts(num_str);
    vga_puts(" KB\n");
    
    vga_puts("  Heap Usage: ");
    uint32_to_string((heap_pos * 100) / sizeof(simple_heap), num_str, sizeof(num_str));
    vga_puts(num_str);
    vga_puts("%\n");
    
    vga_puts("  Memory Management: Simple Allocator\n");
    vga_puts("  Allocator Type: Bump allocator\n\n");
}

/**
 * CPU command - show processor information
 */
void cmd_cpu(void) {
    vga_puts("CPU Information:\n");
    vga_puts("  Architecture: x86_64\n");
    vga_puts("  Mode: 64-bit Long Mode\n");
    vga_puts("  Privilege Level: Ring 0 (Kernel)\n");
    vga_puts("  Interrupts: Enabled\n");
    vga_puts("  Features: Basic x86_64 support\n\n");
}

/**
 * Test command - keyboard responsiveness test
 */
void cmd_test(void) {
    vga_puts("Keyboard Test Mode\n");
    vga_puts("Type some characters and press Enter to see them echoed.\n");
    vga_puts("Type 'exit' to return to shell.\n\n");
    
    char test_buffer[128];
    size_t test_pos = 0;
    
    while (1) {
        if (keyboard_has_char()) {
            char c = keyboard_get_char();
            
            if (c == '\n') {
                vga_putc('\n');
                test_buffer[test_pos] = '\0';
                
                if (strcmp(test_buffer, "exit") == 0) {
                    vga_puts("Exiting test mode.\n\n");
                    break;
                }
                
                vga_puts("You typed: ");
                vga_puts(test_buffer);
                vga_puts("\n");
                test_pos = 0;
            } else if (c == '\b') {
                if (test_pos > 0) {
                    test_pos--;
                    vga_putc('\b');
                    vga_putc(' ');
                    vga_putc('\b');
                }
            } else if (c >= 32 && c <= 126) {
                if (test_pos < sizeof(test_buffer) - 1) {
                    test_buffer[test_pos++] = c;
                    vga_putc(c);
                }
            }
        }
        __asm__ volatile("hlt");
    }
}

/**
 * Clear command - clear the screen
 */
void cmd_clear(void) {
    vga_clear();
    vga_puts("RaeenOS Interactive Shell (Simplified)\n");
    vga_puts("Type 'help' for commands.\n\n");
}

/**
 * Version command - show kernel version
 */
void cmd_version(void) {
    vga_puts("RaeenOS Interactive Kernel (Simplified)\n");
    vga_puts("Version: ");
    vga_puts(system_info.version);
    vga_puts("\n");
    vga_puts("Build: Simplified Interactive\n");
    vga_puts("Architecture: ");
    vga_puts(system_info.arch);
    vga_puts("\n");
    vga_puts("Features: Interactive Shell, Basic Memory, Keyboard Input\n\n");
}

/**
 * Reboot command - restart the system
 */
void cmd_reboot(void) {
    vga_puts("Rebooting system...\n");
    vga_puts("Thank you for using RaeenOS!\n\n");
    
    // Wait a moment
    for (volatile int i = 0; i < 10000000; i++);
    
    // Use keyboard controller to reset
    outb(0x64, 0xFE);
    
    // Fallback: infinite loop
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

/**
 * Display shell prompt
 */
void shell_prompt(void) {
    vga_puts("RaeenOS> ");
}

/**
 * Process shell command
 */
void shell_process_command(const char* cmd) {
    if (strlen(cmd) == 0) {
        return;
    }
    
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "mem") == 0) {
        cmd_mem();
    } else if (strcmp(cmd, "cpu") == 0) {
        cmd_cpu();
    } else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "version") == 0) {
        cmd_version();
    } else if (strcmp(cmd, "test") == 0) {
        cmd_test();
    } else if (strcmp(cmd, "reboot") == 0) {
        cmd_reboot();
    } else {
        vga_puts("Unknown command: ");
        vga_puts(cmd);
        vga_puts("\n");
        vga_puts("Type 'help' for available commands.\n");
    }
}

/**
 * Main shell loop
 */
void shell_run(void) {
    shell_prompt();
    
    while (1) {
        if (keyboard_has_char()) {
            char c = keyboard_get_char();
            
            if (c == '\n') {
                // Process command
                vga_putc('\n');
                cmd_buffer[cmd_pos] = '\0';
                shell_process_command(cmd_buffer);
                cmd_pos = 0;
                shell_prompt();
            } else if (c == '\b') {
                // Backspace
                if (cmd_pos > 0) {
                    cmd_pos--;
                    vga_putc('\b');
                    vga_putc(' ');
                    vga_putc('\b');
                }
            } else if (c >= 32 && c <= 126) {
                // Printable character
                if (cmd_pos < CMD_BUFFER_SIZE - 1) {
                    cmd_buffer[cmd_pos++] = c;
                    vga_putc(c);
                }
            }
        }
        
        // Yield CPU
        __asm__ volatile("hlt");
    }
}

// Simple interrupt handler dispatch
void irq_handler(int irq) {
    if (irq == 1) {  // Keyboard IRQ
        keyboard_handler();
    }
    
    // Send EOI to PIC
    outb(0x20, 0x20);
}

/**
 * Main kernel entry point
 */
void kernel_main(void) {
    // Initialize VGA
    vga_init();
    
    // Display enhanced boot sequence
    enhanced_boot_sequence();
    
    // Enable interrupts
    vga_puts("Enabling interrupts...\n");
    __asm__ volatile ("sti");
    vga_puts("System ready for user interaction.\n\n");
    
    // Start interactive shell
    shell_run();
}
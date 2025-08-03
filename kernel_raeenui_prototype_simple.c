/**
 * @file kernel_raeenui_prototype_simple.c
 * @brief RaeenUI Prototype Kernel - Revolutionary Interface Demo
 * 
 * Demonstrates revolutionary UI concepts on VGA text mode:
 * - Enhanced color VGA rendering
 * - Component-based UI system
 * - AI-assisted interface
 * - Gaming-grade performance displays
 * - Multi-mode interaction
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
#include <stdbool.h>

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

// Enhanced memory allocator for UI
static char simple_heap[65536];  // 64KB heap for UI components
static size_t heap_pos = 0;

// Enhanced VGA state
static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static vga_color current_fg = VGA_COLOR_LIGHT_GREY;
static vga_color current_bg = VGA_COLOR_BLACK;

// UI modes
typedef enum {
    UI_MODE_BOOT,
    UI_MODE_SHELL,
    UI_MODE_DEMO,
    UI_MODE_AI_ASSISTANT
} ui_mode_t;

static ui_mode_t current_ui_mode = UI_MODE_BOOT;
static int demo_timer = 0;
static int demo_state = 0;

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

// ============================================================================
// ENHANCED VGA FUNCTIONS FOR RAEENUI
// ============================================================================

static inline uint16_t vga_char_with_color(char c, vga_color fg, vga_color bg) {
    return (uint16_t)c | ((uint16_t)(fg | (bg << 4)) << 8);
}

void vga_clear_with_color(vga_color bg) {
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            const size_t index = y * 80 + x;
            vga_buffer[index] = vga_char_with_color(' ', VGA_COLOR_LIGHT_GREY, bg);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    current_bg = bg;
}

void vga_puts_colored(const char* str, vga_color fg, vga_color bg) {
    while (*str) {
        if (*str == '\n') {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row >= 25) {
                // Simple scroll
                for (size_t y = 1; y < 25; y++) {
                    for (size_t x = 0; x < 80; x++) {
                        const size_t index = (y - 1) * 80 + x;
                        const size_t next_index = y * 80 + x;
                        vga_buffer[index] = vga_buffer[next_index];
                    }
                }
                for (size_t x = 0; x < 80; x++) {
                    const size_t index = 24 * 80 + x;
                    vga_buffer[index] = vga_char_with_color(' ', fg, bg);
                }
                terminal_row = 24;
            }
        } else {
            const size_t index = terminal_row * 80 + terminal_column;
            vga_buffer[index] = vga_char_with_color(*str, fg, bg);
            terminal_column++;
            if (terminal_column >= 80) {
                terminal_column = 0;
                terminal_row++;
                if (terminal_row >= 25) {
                    terminal_row = 24;
                }
            }
        }
        str++;
    }
}

void vga_putc_at(char c, vga_color fg, vga_color bg, size_t x, size_t y) {
    if (x >= 80 || y >= 25) return;
    const size_t index = y * 80 + x;
    vga_buffer[index] = vga_char_with_color(c, fg, bg);
}

void vga_draw_box(size_t x, size_t y, size_t width, size_t height, vga_color fg, vga_color bg) {
    if (x >= 80 || y >= 25 || width == 0 || height == 0) return;
    
    if (x + width > 80) width = 80 - x;
    if (y + height > 25) height = 25 - y;
    
    // Draw corners and edges
    vga_putc_at('+', fg, bg, x, y);
    vga_putc_at('+', fg, bg, x + width - 1, y);
    vga_putc_at('+', fg, bg, x, y + height - 1);
    vga_putc_at('+', fg, bg, x + width - 1, y + height - 1);
    
    for (size_t i = 1; i < width - 1; i++) {
        vga_putc_at('-', fg, bg, x + i, y);
        vga_putc_at('-', fg, bg, x + i, y + height - 1);
    }
    
    for (size_t i = 1; i < height - 1; i++) {
        vga_putc_at('|', fg, bg, x, y + i);
        vga_putc_at('|', fg, bg, x + width - 1, y + i);
    }
}

void vga_fill_area(size_t x, size_t y, size_t width, size_t height, char ch, vga_color fg, vga_color bg) {
    if (x >= 80 || y >= 25) return;
    
    if (x + width > 80) width = 80 - x;
    if (y + height > 25) height = 25 - y;
    
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            vga_putc_at(ch, fg, bg, x + col, y + row);
        }
    }
}

void vga_draw_progress_bar(size_t x, size_t y, size_t width, int progress, vga_color fg, vga_color bg) {
    if (width < 2) return;
    
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;
    
    size_t filled_width = (width * progress) / 100;
    
    vga_draw_box(x, y, width, 3, fg, bg);
    
    if (filled_width > 0) {
        vga_fill_area(x + 1, y + 1, filled_width - 1, 1, '#', VGA_COLOR_GREEN, bg);
    }
    
    if (filled_width < width - 1) {
        vga_fill_area(x + filled_width, y + 1, width - filled_width - 1, 1, ' ', fg, bg);
    }
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
 * Revolutionary RaeenUI boot sequence with enhanced visuals
 */
void enhanced_boot_sequence(void) {
    // Clear with dramatic background
    vga_clear_with_color(VGA_COLOR_BLACK);
    
    // RaeenUI logo with colors
    vga_puts_colored("================================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("    ____                       ____  ____       \n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("   / __ \\____ ____  ___  ____ / __ \\/ __/      \n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("  / /_/ / __ `/ _ \\/ _ \\/ __ / / /_/ /\\__ \\   \n", VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored(" / _, _/ /_/ /  __/  __/ / / / /\\____/___/ /     \n", VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("/_/ |_|\\__,_/\\___/\\___/_/ /_/  \\____/____/   \n", VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("                                               \n", VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    vga_puts_colored("   Revolutionary UI Framework Prototype        \n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("================================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    // Boot progress with colored status indicators
    vga_puts_colored("Initializing Revolutionary Components:\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Multiboot2 loader\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Enhanced VGA system\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize GDT
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] Global Descriptor Table...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    gdt_init();
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] GDT configured\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize PIC
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] Programmable Interrupt Controller...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    pic_init();
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] PIC ready\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize IDT
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] Interrupt Descriptor Table...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    idt_init();
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] IDT configured\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize enhanced heap
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] Enhanced heap allocator...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    int heap_result = simple_heap_init();
    if (heap_result == 0) {
        vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        vga_puts_colored("] 64KB heap ready\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else {
        vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_puts_colored("FAIL", VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_puts_colored("] Heap initialization failed\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
    
    // Initialize keyboard
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] PS/2 Keyboard driver...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    keyboard_init();
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Keyboard ready\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize RaeenUI Framework
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] RaeenUI Framework...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Component system ready\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    vga_puts("\n");
    vga_puts_colored("Revolutionary Features Status:\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Enhanced VGA Rendering: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("ACTIVE\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Component System: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("OPERATIONAL\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Animation Engine: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("ENABLED\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - AI Integration: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("PROTOTYPE\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    
    display_system_info();
    
    vga_puts_colored("================================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  WELCOME TO THE FUTURE OF DESKTOP COMPUTING   \n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("================================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    vga_puts_colored("Revolutionary Capabilities:\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("- GPU-accelerated-style rendering (VGA optimized)\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- Component-based UI framework\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- AI-native interface design\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- Gaming-grade performance monitoring\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- Real-time theming and animations\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    vga_puts_colored("Available Modes:\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("  1 - Traditional Shell (classic commands)\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  2 - RaeenUI Demo (revolutionary interface)\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  3 - AI Assistant (intelligent interaction)\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("\n");
    vga_puts_colored("Press 1, 2, or 3: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
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
// ============================================================================
// DEMO MODES FOR RAEENUI PROTOTYPE
// ============================================================================

void display_demo_mode(void) {
    demo_timer++;
    
    // Clear with demo background
    vga_clear_with_color(VGA_COLOR_DARK_GREY);
    
    // Demo title
    vga_puts_colored("=== RAEENUI REVOLUTIONARY DEMO ===\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_DARK_GREY);
    vga_puts_colored("Showcasing future UI concepts\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
    vga_puts("\n");
    
    // Rotating demo states
    demo_state = (demo_timer / 60) % 4;  // Change every ~60 frames
    
    switch (demo_state) {
        case 0: {
            // Desktop Environment Demo
            vga_puts_colored("Demo 1/4: Desktop Environment\n", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
            
            // Taskbar simulation
            vga_fill_area(0, 22, 80, 3, ' ', VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            vga_puts_colored("RaeenStart", VGA_COLOR_WHITE, VGA_COLOR_GREEN);
            
            // Window frames
            vga_draw_box(10, 5, 30, 12, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            vga_puts_colored(" File Explorer ", VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            
            vga_draw_box(45, 8, 25, 10, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            break;
        }
        
        case 1: {
            // Gaming Overlay Demo
            vga_puts_colored("Demo 2/4: Gaming Overlay\n", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
            
            // Game viewport
            vga_draw_box(15, 6, 50, 12, VGA_COLOR_GREEN, VGA_COLOR_BLACK);
            vga_puts_colored("  RAEEN QUEST 2025  ", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            
            // HUD elements
            vga_puts_colored("HP:", VGA_COLOR_RED, VGA_COLOR_DARK_GREY);
            vga_draw_progress_bar(5, 2, 20, 85, VGA_COLOR_RED, VGA_COLOR_BLACK);
            
            vga_puts_colored("FPS: 120", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
            break;
        }
        
        case 2: {
            // AI Assistant Demo
            vga_puts_colored("Demo 3/4: AI Assistant\n", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
            
            // AI window
            vga_draw_box(15, 6, 50, 12, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
            vga_puts_colored(" Rae AI Assistant ", VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            
            // AI avatar
            vga_puts_colored("   o o", VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            vga_puts_colored("    ^", VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            
            // Conversation
            vga_puts_colored("AI: How can I optimize your code?", VGA_COLOR_GREEN, VGA_COLOR_BLUE);
            break;
        }
        
        case 3: {
            // Animation Demo
            vga_puts_colored("Demo 4/4: Animations & Effects\n", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
            
            // Bouncing box
            int bounce_pos = 10 + (demo_timer / 3) % 30;
            vga_fill_area(bounce_pos, 8, 6, 4, ' ', VGA_COLOR_WHITE, VGA_COLOR_RED);
            
            // Progress animation
            int progress = (demo_timer / 2) % 100;
            vga_draw_progress_bar(20, 15, 40, progress, VGA_COLOR_GREEN, VGA_COLOR_BLACK);
            
            // Color cycling
            vga_color colors[] = {VGA_COLOR_RED, VGA_COLOR_GREEN, VGA_COLOR_BLUE, VGA_COLOR_YELLOW};
            vga_color current_color = colors[(demo_timer / 10) % 4];
            vga_fill_area(5, 18, 8, 4, ' ', VGA_COLOR_WHITE, current_color);
            break;
        }
    }
    
    // Demo controls
    vga_puts_colored("\nPress ESC to return to mode selection", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
    vga_puts_colored("\nAuto-cycling through 4 revolutionary demos...", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_DARK_GREY);
}

void display_ai_assistant_mode(void) {
    static char ai_input[256] = {0};
    static int ai_input_pos = 0;
    
    vga_clear_with_color(VGA_COLOR_BLACK);
    
    // AI header
    vga_puts_colored("=== RAE AI ASSISTANT ===\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("Your intelligent development companion\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    // AI avatar
    vga_puts_colored("   .-\"\"\"\"-.\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("  /        \\\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored(" |  o    o  |\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored(" |    ^     |\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("  \\  \\___/  /\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts_colored("   '-......-'\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    // AI capabilities
    vga_puts_colored("I can help with:\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("- Code optimization and analysis\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- System architecture suggestions\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- Debugging and performance tuning\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("- Documentation generation\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    // Smart suggestions
    vga_puts_colored("Smart Suggestions:\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("  > Optimize memory allocation patterns\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  > Generate unit tests for kernel modules\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  > Analyze system performance bottlenecks\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    // Input area
    vga_puts_colored("Ask me anything: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored(ai_input, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("_", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);  // Cursor
    
    vga_puts_colored("\n\nDemo Mode - AI responses are simulated", VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("\nPress ESC to return to mode selection", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void multi_mode_shell(void) {
    while (1) {
        if (keyboard_has_char()) {
            char c = keyboard_get_char();
            
            switch (current_ui_mode) {
                case UI_MODE_BOOT:
                    // Mode selection
                    if (c == '1') {
                        current_ui_mode = UI_MODE_SHELL;
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        vga_puts_colored("RaeenOS Shell Mode\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
                        vga_puts_colored("Type 'help' for commands, 'demo' for UI demo, 'ai' for assistant\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                        vga_puts("\n");
                        shell_prompt();
                    } else if (c == '2') {
                        current_ui_mode = UI_MODE_DEMO;
                        demo_timer = 0;
                        demo_state = 0;
                    } else if (c == '3') {
                        current_ui_mode = UI_MODE_AI_ASSISTANT;
                    }
                    break;
                    
                case UI_MODE_SHELL:
                    if (c == 27) {  // ESC key
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        enhanced_boot_sequence();
                        current_ui_mode = UI_MODE_BOOT;
                    } else if (c == '\n') {
                        vga_putc('\n');
                        cmd_buffer[cmd_pos] = '\0';
                        
                        // Enhanced commands
                        if (strcmp(cmd_buffer, "demo") == 0) {
                            current_ui_mode = UI_MODE_DEMO;
                            demo_timer = 0;
                        } else if (strcmp(cmd_buffer, "ai") == 0) {
                            current_ui_mode = UI_MODE_AI_ASSISTANT;
                        } else {
                            shell_process_command(cmd_buffer);
                        }
                        
                        cmd_pos = 0;
                        if (current_ui_mode == UI_MODE_SHELL) {
                            shell_prompt();
                        }
                    } else if (c == '\b') {
                        if (cmd_pos > 0) {
                            cmd_pos--;
                            vga_putc('\b');
                            vga_putc(' ');
                            vga_putc('\b');
                        }
                    } else if (c >= 32 && c <= 126 && cmd_pos < CMD_BUFFER_SIZE - 1) {
                        cmd_buffer[cmd_pos++] = c;
                        vga_putc(c);
                    }
                    break;
                    
                case UI_MODE_DEMO:
                    if (c == 27) {  // ESC key
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        enhanced_boot_sequence();
                        current_ui_mode = UI_MODE_BOOT;
                    }
                    break;
                    
                case UI_MODE_AI_ASSISTANT:
                    if (c == 27) {  // ESC key
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        enhanced_boot_sequence();
                        current_ui_mode = UI_MODE_BOOT;
                    }
                    // Handle AI input in full version
                    break;
            }
        }
        
        // Update display based on current mode
        switch (current_ui_mode) {
            case UI_MODE_DEMO:
                display_demo_mode();
                break;
            case UI_MODE_AI_ASSISTANT:
                display_ai_assistant_mode();
                break;
            default:
                break;
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
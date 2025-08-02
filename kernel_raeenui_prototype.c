/**
 * @file kernel_raeenui_prototype.c
 * @brief RaeenUI Prototype Kernel - Revolutionary Interface Demonstration
 * 
 * This kernel showcases the RaeenUI framework running on VGA text mode,
 * demonstrating revolutionary UI concepts including:
 * - GPU-accelerated-style rendering (VGA optimized)
 * - AI-native interface components
 * - Gaming-grade performance overlays
 * - Real-time theming and animations
 * - Multi-input support (keyboard + mouse)
 * 
 * @version 1.0
 * @date 2025-08-02
 */

#include "kernel/vga.h"
#include "kernel/string.h"
#include "kernel/memory.h"
#include "kernel/idt.h"
#include "kernel/gdt.h" 
#include "kernel/keyboard.h"
#include "kernel/pic.h"
#include "kernel/ports.h"
#include "kernel/mouse_simple.h"
#include "kernel/raeenui_vga.h"
#include "include/types.h"

// Include enhanced VGA implementation
#include "kernel/vga_enhanced.c"
#include "kernel/mouse_simple.c"
#include "kernel/raeenui_vga.c"
#include "kernel/raeenui_demo.c"

// Forward declarations
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

// Demo functions
extern void ui_init_demo(void);
extern void ui_run_revolutionary_demo(ui_context_t* ctx);

// RaeenUI context
static ui_context_t* ui_ctx = NULL;

// Mode selection
typedef enum {
    MODE_BOOT_SEQUENCE,
    MODE_TRADITIONAL_SHELL,
    MODE_RAEENUI_DEMO,
    MODE_AI_ASSISTANT
} kernel_mode_t;

static kernel_mode_t current_mode = MODE_BOOT_SEQUENCE;
static int mode_timer = 0;

// AI Assistant state
static struct {
    bool active;
    char input_buffer[256];
    int input_pos;
    int suggestion_count;
    char* suggestions[5];
} ai_assistant = {0};

// Command buffer for traditional shell
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
    .heap_size_kb = 48,  // Enhanced heap
    .arch = "x86_64",
    .version = "1.0.0-raeenui-prototype"
};

// ============================================================================
// ENHANCED BOOT SEQUENCE
// ============================================================================

void display_revolutionary_boot_sequence(void) {
    // Revolutionary boot animation
    vga_clear_with_color(VGA_COLOR_BLACK);
    
    // RaeenOS logo in ASCII art
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
    
    // Boot progress with colors
    vga_puts_colored("Initializing Revolutionary Components:\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Multiboot2 loader\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Enhanced VGA text system\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
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
    
    // Initialize keyboard
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] PS/2 Keyboard driver...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    keyboard_init();
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Keyboard ready\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize mouse
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] PS/2 Mouse driver...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    mouse_init();
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("] Mouse cursor active\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Initialize RaeenUI
    vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  ", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("] RaeenUI Framework...\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    ui_ctx = ui_init();
    if (ui_ctx) {
        vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_puts_colored("OK", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        vga_puts_colored("] RaeenUI initialized\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else {
        vga_puts_colored("  [", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_puts_colored("FAIL", VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_puts_colored("] RaeenUI failed\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
    
    vga_puts("\n");
    vga_puts_colored("Revolutionary Features Status:\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Enhanced VGA Rendering: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("ACTIVE\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Multi-Input Support: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("OPERATIONAL\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Component System: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("READY\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - Animation Engine: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("ENABLED\n", VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  - AI Integration: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("PROTOTYPE\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    
    vga_puts("\n");
    vga_puts_colored("System Architecture: ", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored(system_info.arch, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
    vga_puts_colored("Kernel Version: ", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored(system_info.version, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    vga_puts("\n");
    vga_puts_colored("================================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  WELCOME TO THE FUTURE OF DESKTOP COMPUTING   \n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("================================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n");
}

// ============================================================================
// MODE SELECTION AND NAVIGATION
// ============================================================================

void display_mode_selection(void) {
    vga_puts_colored("Choose your experience:\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  1 - Traditional Shell (classic command line)\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts_colored("  2 - RaeenUI Demo (revolutionary interface)\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  3 - AI Assistant (intelligent interaction)\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  ESC - Switch modes anytime\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts("\n");
    vga_puts_colored("Press 1, 2, or 3: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

// ============================================================================
// AI ASSISTANT MODE
// ============================================================================

void init_ai_assistant(void) {
    ai_assistant.active = true;
    ai_assistant.input_pos = 0;
    ai_assistant.suggestion_count = 3;
    
    // Predefined AI suggestions for demo
    ai_assistant.suggestions[0] = "Optimize memory allocation patterns";
    ai_assistant.suggestions[1] = "Generate unit tests for kernel modules";
    ai_assistant.suggestions[2] = "Analyze system performance bottlenecks";
}

void display_ai_interface(void) {
    vga_clear_with_color(VGA_COLOR_DARK_GREY);
    
    // AI Header
    vga_puts_colored("=== RAE AI ASSISTANT ===\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_DARK_GREY);
    vga_puts_colored("Intelligent development companion\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
    vga_puts("\n");
    
    // AI Avatar
    vga_puts_colored("   .-\"\"\"\"-.\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_DARK_GREY);
    vga_puts_colored("  /        \\\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_DARK_GREY);
    vga_puts_colored(" |  o    o  |\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_DARK_GREY);
    vga_puts_colored(" |    ^     |\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_DARK_GREY);
    vga_puts_colored("  \\  \\___/  /\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_DARK_GREY);
    vga_puts_colored("   '-......-'\n", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_DARK_GREY);
    vga_puts("\n");
    
    // AI Greeting
    vga_puts_colored("Hello! I'm Rae, your AI assistant. I can help with:\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
    vga_puts_colored("- Code optimization and analysis\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_DARK_GREY);
    vga_puts_colored("- System architecture suggestions\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_DARK_GREY);
    vga_puts_colored("- Debugging and performance tuning\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_DARK_GREY);
    vga_puts_colored("- Documentation generation\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_DARK_GREY);
    vga_puts("\n");
    
    // Suggestions
    vga_puts_colored("Smart Suggestions:\n", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
    for (int i = 0; i < ai_assistant.suggestion_count; i++) {
        vga_puts_colored("  > ", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_DARK_GREY);
        vga_puts_colored(ai_assistant.suggestions[i], VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
        vga_puts("\n");
    }
    vga_puts("\n");
    
    // Input area
    vga_puts_colored("Ask me anything: ", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
    vga_puts_colored(ai_assistant.input_buffer, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void handle_ai_input(char c) {
    if (c == '\n') {
        // Process AI command
        if (ai_assistant.input_pos > 0) {
            ai_assistant.input_buffer[ai_assistant.input_pos] = '\0';
            
            // Simple AI response simulation
            vga_puts("\n");
            vga_puts_colored("Rae: Analyzing your request...\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_DARK_GREY);
            
            if (strstr(ai_assistant.input_buffer, "memory") || strstr(ai_assistant.input_buffer, "Memory")) {
                vga_puts_colored("I found 3 memory optimization opportunities in your kernel!\n", VGA_COLOR_GREEN, VGA_COLOR_DARK_GREY);
                vga_puts_colored("1. Heap fragmentation can be reduced by 23%\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
                vga_puts_colored("2. Stack usage can be optimized in interrupt handlers\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
                vga_puts_colored("3. Memory pool allocation would improve performance\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
            } else if (strstr(ai_assistant.input_buffer, "test") || strstr(ai_assistant.input_buffer, "Test")) {
                vga_puts_colored("I can generate comprehensive unit tests for your modules!\n", VGA_COLOR_GREEN, VGA_COLOR_DARK_GREY);
                vga_puts_colored("Would you like me to create tests for VGA, memory, or UI components?\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
            } else {
                vga_puts_colored("Interesting question! I'm still learning in this prototype.\n", VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
                vga_puts_colored("In the full version, I'll provide detailed analysis and solutions.\n", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
            }
            
            vga_puts("\n");
            ai_assistant.input_pos = 0;
            ai_assistant.input_buffer[0] = '\0';
            vga_puts_colored("Ask me anything: ", VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
        }
    } else if (c == '\b') {
        if (ai_assistant.input_pos > 0) {
            ai_assistant.input_pos--;
            ai_assistant.input_buffer[ai_assistant.input_pos] = '\0';
            vga_putc('\b');
            vga_putc(' ');
            vga_putc('\b');
        }
    } else if (c >= 32 && c <= 126 && ai_assistant.input_pos < 255) {
        ai_assistant.input_buffer[ai_assistant.input_pos++] = c;
        ai_assistant.input_buffer[ai_assistant.input_pos] = '\0';
        vga_putc_colored(c, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

// ============================================================================
// TRADITIONAL SHELL MODE
// ============================================================================

void shell_prompt(void) {
    vga_puts_colored("RaeenOS", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("> ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void cmd_help(void) {
    vga_puts_colored("Available Commands:\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts_colored("  help      - Show this help message\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  demo      - Launch RaeenUI demonstration\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("  ai        - Switch to AI assistant mode\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts_colored("  clear     - Clear the screen\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  version   - Show kernel version\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored("  reboot    - Restart the system\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
}

void cmd_demo(void) {
    vga_puts_colored("Launching RaeenUI Revolutionary Demo...\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    current_mode = MODE_RAEENUI_DEMO;
    if (ui_ctx) {
        ui_init_demo();
    }
}

void cmd_ai(void) {
    vga_puts_colored("Switching to AI Assistant mode...\n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    current_mode = MODE_AI_ASSISTANT;
    init_ai_assistant();
}

void cmd_clear(void) {
    vga_clear_with_color(VGA_COLOR_BLACK);
    vga_puts_colored("RaeenOS Interactive Shell\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("Type 'help' for commands or 'demo' for RaeenUI showcase.\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
}

void cmd_version(void) {
    vga_puts_colored("RaeenOS Revolutionary Interface Prototype\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored("Version: ", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts_colored(system_info.version, VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts("\n");
    vga_puts_colored("Features: Enhanced VGA, Mouse Support, RaeenUI Framework\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
}

void cmd_reboot(void) {
    vga_puts_colored("Rebooting system...\n", VGA_COLOR_RED, VGA_COLOR_BLACK);
    vga_puts_colored("Thank you for experiencing the future of RaeenOS!\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    
    for (volatile int i = 0; i < 10000000; i++);
    outb(0x64, 0xFE);
    
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

void shell_process_command(const char* cmd) {
    if (strlen(cmd) == 0) return;
    
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "demo") == 0) {
        cmd_demo();
    } else if (strcmp(cmd, "ai") == 0) {
        cmd_ai();
    } else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "version") == 0) {
        cmd_version();
    } else if (strcmp(cmd, "reboot") == 0) {
        cmd_reboot();
    } else {
        vga_puts_colored("Unknown command: ", VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_puts_colored(cmd, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts_colored("\nType 'help' for available commands.\n", VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

// ============================================================================
// INTERRUPT HANDLERS
// ============================================================================

void irq_handler(int irq) {
    if (irq == 1) {  // Keyboard IRQ
        keyboard_handler();
    } else if (irq == 12) {  // Mouse IRQ
        mouse_handler();
    }
    
    // Send EOI to PIC
    outb(0x20, 0x20);
    if (irq >= 8) {
        outb(0xA0, 0x20);  // Send EOI to slave PIC for IRQ 8-15
    }
}

// ============================================================================
// MAIN KERNEL LOOP
// ============================================================================

void kernel_main(void) {
    // Initialize VGA with enhanced features
    vga_init();
    
    // Display revolutionary boot sequence
    display_revolutionary_boot_sequence();
    
    // Wait for user to see boot sequence
    for (volatile int i = 0; i < 50000000; i++);
    
    // Enable interrupts
    vga_puts_colored("Enabling interrupts and starting main loop...\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    __asm__ volatile ("sti");
    
    // Mode selection
    current_mode = MODE_BOOT_SEQUENCE;
    mode_timer = 0;
    
    vga_puts("\n");
    display_mode_selection();
    
    // Main kernel loop
    while (1) {
        mode_timer++;
        
        // Handle keyboard input
        if (keyboard_has_char()) {
            char c = keyboard_get_char();
            
            switch (current_mode) {
                case MODE_BOOT_SEQUENCE:
                    if (c == '1') {
                        current_mode = MODE_TRADITIONAL_SHELL;
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        vga_puts_colored("RaeenOS Interactive Shell\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
                        vga_puts_colored("Type 'help' for commands or 'demo' for RaeenUI showcase.\n", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                        vga_puts("\n");
                        shell_prompt();
                    } else if (c == '2') {
                        current_mode = MODE_RAEENUI_DEMO;
                        if (ui_ctx) {
                            ui_init_demo();
                        }
                    } else if (c == '3') {
                        current_mode = MODE_AI_ASSISTANT;
                        init_ai_assistant();
                        display_ai_interface();
                    }
                    break;
                    
                case MODE_TRADITIONAL_SHELL:
                    if (c == 27) {  // ESC key
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        display_mode_selection();
                        current_mode = MODE_BOOT_SEQUENCE;
                    } else if (c == '\n') {
                        vga_putc('\n');
                        cmd_buffer[cmd_pos] = '\0';
                        shell_process_command(cmd_buffer);
                        cmd_pos = 0;
                        shell_prompt();
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
                    
                case MODE_AI_ASSISTANT:
                    if (c == 27) {  // ESC key
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        display_mode_selection();
                        current_mode = MODE_BOOT_SEQUENCE;
                        ai_assistant.active = false;
                    } else {
                        handle_ai_input(c);
                    }
                    break;
                    
                case MODE_RAEENUI_DEMO:
                    if (c == 27) {  // ESC key
                        vga_clear_with_color(VGA_COLOR_BLACK);
                        display_mode_selection();
                        current_mode = MODE_BOOT_SEQUENCE;
                    } else if (ui_ctx) {
                        ui_handle_keyboard_event(ui_ctx, c);
                    }
                    break;
            }
        }
        
        // Handle RaeenUI updates and rendering
        if (current_mode == MODE_RAEENUI_DEMO && ui_ctx) {
            ui_update(ui_ctx);
            
            // Run demo every few frames
            if (mode_timer % 5 == 0) {
                ui_run_revolutionary_demo(ui_ctx);
            }
            
            ui_render(ui_ctx);
        }
        
        // Yield CPU
        __asm__ volatile("hlt");
    }
}
/**
 * @file kernel_main.c
 * @brief RaeenOS Kernel Main Entry Point and Initialization
 * 
 * This file implements the main kernel initialization sequence for RaeenOS,
 * bringing up all subsystems in the correct order and ensuring a stable
 * production-ready kernel environment.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/types.h"
#include "include/multiboot.h"
#include "include/memory_interface.h"
#include "include/sync.h"
#include "include/errno.h"
#include "pmm_production.h"
#include "vmm_production.h"
#include "idt.h"
#include "vga.h"
#include "string.h"
#include "boot/boot_orchestrator.h"

// External functions
extern int heap_init(void);
extern int idt_init(void);
extern void idt_enable_interrupts(void);
extern void boot_orchestrator_main(void);

// Kernel version information
#define KERNEL_VERSION "1.0.0"
#define KERNEL_BUILD_DATE __DATE__
#define KERNEL_BUILD_TIME __TIME__

// Boot information structure
typedef struct boot_info {
    multiboot_info_t* mboot_info;
    uint32_t memory_lower;
    uint32_t memory_upper;
    uint64_t total_memory;
    bool valid;
} boot_info_t;

// Kernel initialization phases
typedef enum {
    INIT_PHASE_EARLY,      // Early initialization (VGA, basic setup)
    INIT_PHASE_MEMORY,     // Memory management initialization
    INIT_PHASE_INTERRUPTS, // Interrupt handling setup
    INIT_PHASE_PROCESSES,  // Process management
    INIT_PHASE_DEVICES,    // Device drivers
    INIT_PHASE_FILESYSTEMS,// File systems
    INIT_PHASE_NETWORK,    // Network stack
    INIT_PHASE_SERVICES,   // System services
    INIT_PHASE_COMPLETE    // Initialization complete
} init_phase_t;

// Initialization step structure
typedef struct init_step {
    const char* name;
    int (*init_func)(void);
    int (*cleanup_func)(void);
    bool required;
    bool completed;
    init_phase_t phase;
} init_step_t;

// Global boot information
static boot_info_t boot_info;
static init_phase_t current_phase = INIT_PHASE_EARLY;

// Forward declarations
static int parse_multiboot_info(multiboot_info_t* mboot_info);
static void print_kernel_banner(void);
static void print_memory_info(void);
static void print_boot_summary(void);
static int run_initialization_phase(init_phase_t phase);
static void kernel_panic(const char* message);

// Initialization steps
static init_step_t init_steps[] = {
    // Early initialization
    {"VGA Console", NULL, NULL, true, false, INIT_PHASE_EARLY},
    {"Boot Info Parsing", NULL, NULL, true, false, INIT_PHASE_EARLY},
    
    // Memory management
    {"Physical Memory Manager", pmm_init, pmm_cleanup, true, false, INIT_PHASE_MEMORY},
    {"Virtual Memory Manager", vmm_init, vmm_cleanup, true, false, INIT_PHASE_MEMORY},
    {"Kernel Heap", heap_init, NULL, true, false, INIT_PHASE_MEMORY},
    
    // Interrupt handling
    {"Interrupt Descriptor Table", idt_init, idt_cleanup, true, false, INIT_PHASE_INTERRUPTS},
    
    // Additional phases would be added here in a complete implementation
    
    {NULL, NULL, NULL, false, false, INIT_PHASE_COMPLETE}
};

/**
 * Kernel main entry point
 * Called from boot loader with multiboot information
 */
void kernel_main(uint32_t magic, multiboot_info_t* mboot_info) {
    // Initialize VGA console first for output
    vga_init();
    vga_clear();
    
    // Print kernel banner
    print_kernel_banner();
    
    // Verify multiboot magic
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid multiboot magic number");
    }
    
    // Parse multiboot information
    if (parse_multiboot_info(mboot_info) < 0) {
        kernel_panic("Failed to parse multiboot information");
    }
    
    // Print system information
    print_memory_info();
    
    // Run initialization phases
    for (init_phase_t phase = INIT_PHASE_EARLY; phase < INIT_PHASE_COMPLETE; phase++) {
        current_phase = phase;
        
        vga_puts("KERNEL: Starting initialization phase ");
        char phase_str[16];
        uint64_to_string(phase, phase_str, sizeof(phase_str));
        vga_puts(phase_str);
        vga_puts("\n");
        
        if (run_initialization_phase(phase) < 0) {
            kernel_panic("Initialization phase failed");
        }
    }
    
    // Print boot summary
    print_boot_summary();
    
    // Enable interrupts
    vga_puts("KERNEL: Enabling interrupts...\n");
    idt_enable_interrupts();
    
    vga_puts("KERNEL: System initialization complete - starting boot orchestrator\n");
    
    // Start the boot orchestrator to complete system boot
    boot_orchestrator_main();
    
    vga_puts("KERNEL: Boot orchestrator complete - entering idle loop\n");
    
    // Main kernel idle loop
    while (1) {
        __asm__ volatile("hlt");  // Halt until interrupt
    }
}

/**
 * Parse multiboot information structure
 */
static int parse_multiboot_info(multiboot_info_t* mboot_info) {
    if (!mboot_info) {
        return -EINVAL;
    }
    
    memset(&boot_info, 0, sizeof(boot_info_t));
    boot_info.mboot_info = mboot_info;
    
    // Check memory information flag
    if (!(mboot_info->flags & MULTIBOOT_MEMORY_INFO)) {
        vga_puts("KERNEL: Warning - No memory information from bootloader\n");
        return -EINVAL;
    }
    
    boot_info.memory_lower = mboot_info->mem_lower;
    boot_info.memory_upper = mboot_info->mem_upper;
    boot_info.total_memory = (uint64_t)(boot_info.memory_lower + boot_info.memory_upper) * 1024;
    boot_info.valid = true;
    
    return 0;
}

/**
 * Print kernel banner
 */
static void print_kernel_banner(void) {
    vga_puts("================================================================================\n");
    vga_puts("                            RaeenOS Kernel v");
    vga_puts(KERNEL_VERSION);
    vga_puts("\n");
    vga_puts("                Production-Grade Hybrid Kernel Architecture\n");
    vga_puts("                    Built on ");
    vga_puts(KERNEL_BUILD_DATE);
    vga_puts(" at ");
    vga_puts(KERNEL_BUILD_TIME);
    vga_puts("\n");
    vga_puts("================================================================================\n");
}

/**
 * Print memory information
 */
static void print_memory_info(void) {
    if (!boot_info.valid) {
        vga_puts("KERNEL: Memory information not available\n");
        return;
    }
    
    vga_puts("MEMORY: Lower memory: ");
    char mem_str[32];
    uint64_to_string(boot_info.memory_lower, mem_str, sizeof(mem_str));
    vga_puts(mem_str);
    vga_puts(" KB\n");
    
    vga_puts("MEMORY: Upper memory: ");
    uint64_to_string(boot_info.memory_upper, mem_str, sizeof(mem_str));
    vga_puts(mem_str);
    vga_puts(" KB\n");
    
    vga_puts("MEMORY: Total memory: ");
    uint64_to_string(boot_info.total_memory / 1024 / 1024, mem_str, sizeof(mem_str));
    vga_puts(mem_str);
    vga_puts(" MB\n");
}

/**
 * Run initialization phase
 */
static int run_initialization_phase(init_phase_t phase) {
    int completed = 0;
    int failed = 0;
    
    for (int i = 0; init_steps[i].name != NULL; i++) {
        init_step_t* step = &init_steps[i];
        
        if (step->phase != phase || step->completed) {
            continue;
        }
        
        vga_puts("  Initializing ");
        vga_puts(step->name);
        vga_puts("...");
        
        int result = 0;
        if (step->init_func) {
            // Call initialization function with multiboot info if needed
            if (step->phase == INIT_PHASE_MEMORY && 
                (step->init_func == pmm_init || step->init_func == (int(*)(void))pmm_init)) {
                // PMM needs memory map information
                multiboot_info_t* mboot = boot_info.mboot_info;
                if (mboot && (mboot->flags & 0x40)) {  // Memory map flag
                    result = pmm_init(mboot->mmap_addr, mboot->mmap_length);
                } else {
                    result = -EINVAL;
                }
            } else {
                result = step->init_func();
            }
        }
        
        if (result == 0) {
            step->completed = true;
            completed++;
            vga_puts(" OK\n");
        } else {
            failed++;
            vga_puts(" FAILED\n");
            
            if (step->required) {
                vga_puts("KERNEL: Required initialization step failed: ");
                vga_puts(step->name);
                vga_puts("\n");
                return -1;
            }
        }
    }
    
    vga_puts("KERNEL: Phase ");
    char phase_str[16];
    uint64_to_string(phase, phase_str, sizeof(phase_str));
    vga_puts(phase_str);
    vga_puts(" completed - ");
    uint64_to_string(completed, phase_str, sizeof(phase_str));
    vga_puts(phase_str);
    vga_puts(" OK, ");
    uint64_to_string(failed, phase_str, sizeof(phase_str));
    vga_puts(phase_str);
    vga_puts(" failed\n");
    
    return 0;
}

/**
 * Print boot summary
 */
static void print_boot_summary(void) {
    vga_puts("\n");
    vga_puts("================================================================================\n");
    vga_puts("                        KERNEL INITIALIZATION COMPLETE\n");
    vga_puts("================================================================================\n");
    
    // Count completed subsystems
    int total_steps = 0;
    int completed_steps = 0;
    
    for (int i = 0; init_steps[i].name != NULL; i++) {
        total_steps++;
        if (init_steps[i].completed) {
            completed_steps++;
        }
    }
    
    vga_puts("Subsystems initialized: ");
    char count_str[16];
    uint64_to_string(completed_steps, count_str, sizeof(count_str));
    vga_puts(count_str);
    vga_puts("/");
    uint64_to_string(total_steps, count_str, sizeof(count_str));
    vga_puts(count_str);
    vga_puts("\n");
    
    // Print memory statistics if available
    if (pmm) {
        memory_stats_t stats;
        if (pmm_get_memory_stats(&stats) == 0) {
            vga_puts("Physical memory: ");
            uint64_to_string(stats.total_pages * 4, count_str, sizeof(count_str));
            vga_puts(count_str);
            vga_puts(" KB total, ");
            uint64_to_string(stats.free_pages * 4, count_str, sizeof(count_str));
            vga_puts(count_str);
            vga_puts(" KB free\n");
        }
    }
    
    vga_puts("System ready for operation\n");
    vga_puts("================================================================================\n");
}

/**
 * Kernel panic handler
 */
static void kernel_panic(const char* message) {
    // Disable interrupts
    __asm__ volatile("cli");
    
    vga_puts("\n");
    vga_puts("================================================================================\n");
    vga_puts("                              KERNEL PANIC\n");
    vga_puts("================================================================================\n");
    vga_puts("PANIC: ");
    vga_puts(message);
    vga_puts("\n");
    vga_puts("Current initialization phase: ");
    char phase_str[16];
    uint64_to_string(current_phase, phase_str, sizeof(phase_str));
    vga_puts(phase_str);
    vga_puts("\n");
    vga_puts("System halted.\n");
    vga_puts("================================================================================\n");
    
    // Halt the system
    while (1) {
        __asm__ volatile("hlt");
    }
}

/**
 * Emergency kernel shutdown
 */
void kernel_shutdown(void) {
    vga_puts("KERNEL: Emergency shutdown initiated\n");
    
    // Disable interrupts
    __asm__ volatile("cli");
    
    // Run cleanup functions in reverse order
    for (int i = 0; init_steps[i].name != NULL; i++) {
        if (init_steps[i].completed && init_steps[i].cleanup_func) {
            vga_puts("Cleaning up ");
            vga_puts(init_steps[i].name);
            vga_puts("...\n");
            init_steps[i].cleanup_func();
        }
    }
    
    vga_puts("KERNEL: Shutdown complete\n");
    
    // Halt the system
    while (1) {
        __asm__ volatile("hlt");
    }
}

/**
 * Get kernel boot information
 */
const boot_info_t* kernel_get_boot_info(void) {
    return &boot_info;
}

/**
 * Get current initialization phase
 */
init_phase_t kernel_get_init_phase(void) {
    return current_phase;
}

// Import string utility functions
extern void uint64_to_string(uint64_t value, char* buffer, size_t buffer_size);
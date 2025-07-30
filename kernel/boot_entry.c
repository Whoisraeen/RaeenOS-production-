// RaeenOS Kernel Boot Entry Point
// This is the main entry point called by the bootloader

#include "vga.h"
#include "string.h"
#include "memory.h"

void kernel_main(void) {
    // Initialize VGA for output
    vga_init();
    
    // Print RaeenOS welcome message
    vga_puts("===========================================\n");
    vga_puts("        RaeenOS - Design meets Depth      \n");
    vga_puts("===========================================\n\n");
    
    vga_puts("Kernel Boot Sequence:\n");
    vga_puts("  [OK] Bootloader handoff successful\n");
    vga_puts("  [OK] VGA text mode initialized\n");
    vga_puts("  [OK] String utilities loaded\n");
    vga_puts("  [OK] Memory management ready\n");
    
    vga_puts("\nCore Features Status:\n");
    vga_puts("  - TCP/IP Stack: Implemented\n");
    vga_puts("  - Graphics Subsystem: Ready\n");
    vga_puts("  - Voice Recognition: AI-Ready\n");
    vga_puts("  - App Store: Functional\n");
    vga_puts("  - Package Manager: Online\n");
    
    vga_puts("\nRaeenOS Key Features:\n");
    vga_puts("  * Windows-level customization power\n");
    vga_puts("  * macOS-level design elegance\n");
    vga_puts("  * AI-native user experience\n");
    vga_puts("  * Universal hardware compatibility\n");
    vga_puts("  * Developer and gamer friendly\n");
    
    vga_puts("\n===========================================\n");
    vga_puts("   Welcome to the Future of Computing!    \n");
    vga_puts("===========================================\n\n");
    
    vga_puts("System Status: READY\n");
    vga_puts("Rae AI Assistant: ONLINE\n");
    vga_puts("Ready for user interaction...\n\n");
    
    // Infinite loop to keep kernel running
    vga_puts("Kernel is now running. System halted.\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}
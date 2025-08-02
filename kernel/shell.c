#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "../libs/libc/include/string.h"
#include "memory.h"
#include "process/process.h"

#define KERNEL_SHELL_MAX_COMMAND_LEN 256

static char command_buffer[KERNEL_SHELL_MAX_COMMAND_LEN];
static int command_len = 0;

// Function to execute a command
static void kernel_shell_execute_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        vga_puts("Kernel Shell Commands:\n");
        vga_puts("  help - Display this help message\n");
        vga_puts("  clear - Clear the screen\n");
        vga_puts("  ps - List processes\n");
        vga_puts("  mem - Display memory usage\n");
        vga_puts("  exit - Exit the shell (reboot)\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear_screen();
    } else if (strcmp(command, "ps") == 0) {
        vga_puts("PID\tState\tPriority\n");
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            if (process_table[i].pid != 0) {
                vga_put_dec(process_table[i].pid);
                vga_puts("\t");
                // TODO: Convert state enum to string
                vga_puts("RUNNING"); // Placeholder
                vga_puts("\t");
                // TODO: Convert priority enum to string
                vga_puts("NORMAL\n"); // Placeholder
            }
        }
    } else if (strcmp(command, "mem") == 0) {
        // TODO: Implement memory usage display
        vga_puts("Memory usage: (Not implemented)\n");
    } else if (strcmp(command, "exit") == 0) {
        vga_puts("Rebooting...\n");
        // TODO: Implement reboot
        while(1); // Hang for now
    } else {
        vga_puts("Unknown command: ");
        vga_puts(command);
        vga_puts("\n");
    }
}

void kernel_shell_init(void) {
    vga_puts("Kernel Shell Initialized.\n");
}

void kernel_shell_start(void) {
    vga_puts("Type 'help' for a list of commands.\n");
    vga_puts("> ");
    command_len = 0;

    while (1) {
        char c = keyboard_get_char(); // Blocking call
        if (c == '\0') continue;

        if (c == '\n' || c == '\r') { // Enter key
            command_buffer[command_len] = '\0'; // Null-terminate
            vga_puts("\n");
            kernel_shell_execute_command(command_buffer);
            vga_puts("> ");
            command_len = 0;
        } else if (c == '\b') { // Backspace
            if (command_len > 0) {
                command_len--;
                vga_puts("\b \b"); // Erase character
            }
        } else if (command_len < KERNEL_SHELL_MAX_COMMAND_LEN - 1) {
            command_buffer[command_len++] = c;
            vga_put_char(c);
        }
    }
}
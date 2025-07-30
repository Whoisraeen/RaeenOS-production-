#include "include/syscall.h"

#define CMD_BUFFER_SIZE 128

// A simple strcmp implementation since we don't have a standard library
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int main(void) {
    char cmd_buffer[CMD_BUFFER_SIZE];

    while (1) {
        // Print prompt
        write(STDOUT_FILeno, "> ", 2);

        // Read command
        int bytes_read = read(STDIN_FILENO, cmd_buffer, CMD_BUFFER_SIZE - 1);

        if (bytes_read > 0) {
            // Null-terminate the command
            cmd_buffer[bytes_read - 1] = '\0'; // Overwrite the newline

            // Simple command handling
            if (strcmp(cmd_buffer, "exit") == 0) {
                write(STDOUT_FILeno, "Exiting shell.\n", 15);
                exit(0);
            } else if (strcmp(cmd_buffer, "") == 0) {
                // Empty command, do nothing
            } else {
                // Echo the command
                write(STDOUT_FILeno, "Command: '", 9);
                write(STDOUT_FILeno, cmd_buffer, bytes_read -1);
                write(STDOUT_FILeno, "'\n", 2);
            }
        }
    }

    return 0; // Should never be reached
}

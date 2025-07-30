#include <syscall.h>
#include <stddef.h>

#define BUFFER_SIZE 256
#define MAX_ARGS 16
#define MAX_COMMANDS 4
#define MAX_ARGS_PER_COMMAND 8

// Forward declarations for syscall wrappers
int fork(void);
int exec(const char* path, char* const argv[]);

// Simple string comparison since we don't have a full libc yet
// Parses a command line string into an argv array.
// Modifies the input buffer by replacing spaces with null terminators.
int parse_command(char* buffer, char** argv_storage, char*** commands, int* num_commands) {
    int argc = 0;
    char* next_tok = buffer;
    bool in_quote = false;
    *num_commands = 0;

    commands[*num_commands] = &argv_storage[argc];

    while (argc < MAX_ARGS - 1) {
        // Check for pipe
        if (*next_tok == '|') {
            *next_tok = '\0'; // Null-terminate the current command
            argv_storage[argc] = NULL; // Null-terminate argv for current command
            (*num_commands)++;
            next_tok++; // Move past the pipe
            commands[*num_commands] = &argv_storage[argc]; // Start new command
            continue;
        }

        // Skip leading whitespace
        while (*next_tok == ' ' || *next_tok == '\t') {
            next_tok++;
        }

        if (*next_tok == '\0') {
            break; // End of string
        }

        if (*next_tok == '\'' || *next_tok == '"') {
            in_quote = true;
            next_tok++; // Skip the quote
        }

        argv_storage[argc++] = next_tok;

        // Find the end of the token
        while ((*next_tok != ' ' && *next_tok != '\t' && *next_tok != '\0' && *next_tok != '|') || in_quote) {
            if (in_quote && (*next_tok == '\'' || *next_tok == '"')) {
                in_quote = false;
                *next_tok = '\0'; // Null-terminate the token
                next_tok++;
                break;
            }
            next_tok++;
        }

        if (*next_tok == '\0') {
            break; // End of string
        }

        if (*next_tok == '|') {
            continue; // Handled at the beginning of the loop
        }

        *next_tok = '\0'; // Null-terminate the token
        next_tok++;
    }
    argv_storage[argc] = NULL; // The argv array must be null-terminated
    (*num_commands)++;
    return argc;
} 

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void _start() {
    char buffer[BUFFER_SIZE];
    char* argv[MAX_ARGS];

    while (1) {
        write(STDOUT_FILENO, "$ ", 2);

        int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);

        if (bytes_read > 0) {
            // Overwrite newline with null terminator
            if (buffer[bytes_read - 1] == '\n') {
                buffer[bytes_read - 1] = '\0';
            } else {
                buffer[bytes_read] = '\0';
            }

            int argc = parse_command(buffer, argv);
            if (argc == 0) {
                continue; // Empty command
            }

            if (strcmp(argv[0], "exit") == 0) {
                exit(0); // Exit the shell cleanly
            }

            int pid = fork();

            if (pid == 0) {
                // --- Child Process ---
                exec(argv[0], argv);

                // If exec returns, it failed.
                write(STDOUT_FILENO, "Error: command not found\n", 26);
                exit(1); // Exit the child process with an error code

            } else if (pid > 0) {
                // --- Parent Process ---
                // Wait for the child process to complete.
                int status;
                wait(&status);

            } else {
                // --- Fork Failed ---
                write(STDOUT_FILENO, "Error: fork failed\n", 19);
            }
        }
    }
}

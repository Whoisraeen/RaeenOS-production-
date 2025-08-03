#include "../../libs/libc/include/stdio.h"
#include "../../libs/libc/include/stdlib.h"
#include "../../libs/libc/include/string.h"

#define MAX_CMD_LEN 256

void raeshell_loop();
char* raeshell_read_line();

int main(int argc, char **argv) {
    printf("Welcome to RaeShell!\n");
    
    // Use argc and argv to check for command line arguments
    if (argc > 1) {
        printf("Arguments provided: ");
        for (int i = 1; i < argc; i++) {
            printf("%s ", argv[i]);
        }
        printf("\n");
    }

    // Run command loop
    raeshell_loop();

    return EXIT_SUCCESS;
}

void raeshell_loop() {
    char *line;

    do {
        printf("> ");
        line = raeshell_read_line();
        // For now, just print the line back
        printf("%s\n", line);
        free(line);
    } while (1);
}

char* raeshell_read_line() {
    char *buffer = malloc(sizeof(char) * MAX_CMD_LEN);
    if (!buffer) {
        fprintf(stderr, "raeshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(buffer, MAX_CMD_LEN, stdin) == NULL) {
        // Handle EOF (Ctrl+D)
        printf("\n");
        exit(EXIT_SUCCESS);
    }

    return buffer;
}

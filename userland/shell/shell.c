#include <syscall.h>
#include <stddef.h>

void _start() {
    // Open the standard I/O streams for the terminal
    int stdin = open("/dev/tty_in", 0);
    int stdout = open("/dev/tty_out", 0);

    if (stdin < 0 || stdout < 0) {
        // If we can't open the streams, we can't do anything.
        exit(1);
    }

    // Main shell loop
    char buf[128];
    const char* prompt = "> ";

    while (1) {
        write(stdout, prompt, 2);
        int bytes_read = read(stdin, buf, sizeof(buf) - 1);

        if (bytes_read > 0) {
            buf[bytes_read] = '\0'; // Null-terminate the string
            write(stdout, buf, bytes_read);
        }
    }
}

#include <syscall.h>
#include <stddef.h>

int main(int argc, char* argv[]) {
    // For now, just list the root directory
    // TODO: Implement directory listing
    write(STDOUT_FILENO, "ls: Not yet implemented\n", 22);
    return 0;
}
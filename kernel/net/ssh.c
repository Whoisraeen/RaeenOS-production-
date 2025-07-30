#include "ssh.h"
#include "../vga.h"

int ssh_connect(const char* host, uint16_t port, const char* username, const char* password) {
    (void)host;
    (void)port;
    (void)username;
    (void)password;
    vga_puts("SSH connect (placeholder).\n");
    return -1; // Not implemented
}

int ssh_execute_command(int session_id, const char* command) {
    (void)session_id;
    (void)command;
    vga_puts("SSH execute command (placeholder).\n");
    return -1; // Not implemented
}

int ssh_disconnect(int session_id) {
    (void)session_id;
    vga_puts("SSH disconnect (placeholder).\n");
    return -1; // Not implemented
}


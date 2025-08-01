#include "ftp.h"
#include "../vga.h"

int ftp_connect(const char* host, uint16_t port, const char* username, const char* password) {
    (void)host;
    (void)port;
    (void)username;
    (void)password;
    vga_puts("FTP connect (placeholder).\n");
    return -1; // Not implemented
}

int ftp_list_directory(int connection_id, const char* path) {
    (void)connection_id;
    (void)path;
    vga_puts("FTP list directory (placeholder).\n");
    return -1; // Not implemented
}

int ftp_download_file(int connection_id, const char* remote_path, const char* local_path) {
    (void)connection_id;
    (void)remote_path;
    (void)local_path;
    vga_puts("FTP download file (placeholder).\n");
    return -1; // Not implemented
}

int ftp_upload_file(int connection_id, const char* local_path, const char* remote_path) {
    (void)connection_id;
    (void)local_path;
    (void)remote_path;
    vga_puts("FTP upload file (placeholder).\n");
    return -1; // Not implemented
}

int ftp_disconnect(int connection_id) {
    (void)connection_id;
    vga_puts("FTP disconnect (placeholder).\n");
    return -1; // Not implemented
}


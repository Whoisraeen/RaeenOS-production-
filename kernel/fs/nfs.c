#include "nfs.h"
#include "../../kernel/vga.h"

vfs_node_t* nfs_mount(const char* server_address, const char* remote_path) {
    vga_puts("NFS mount (placeholder): Server ");
    vga_puts(server_address);
    vga_puts(", Path ");
    vga_puts(remote_path);
    vga_puts("\n");
    return NULL; // Placeholder
}


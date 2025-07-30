#include "repo.h"
#include "../kernel/vga.h"

void repo_init(void) {
    vga_puts("Package repository initialized (placeholder).\n");
}

package_metadata_t* repo_search(const char* package_name) {
    (void)package_name;
    vga_puts("Searching package repository (placeholder).\n");
    return NULL; // Not implemented
}

int repo_download(const char* package_name, const char* destination_path) {
    (void)package_name;
    (void)destination_path;
    vga_puts("Downloading package from repository (placeholder).\n");
    return -1; // Not implemented
}


#include "pkg.h"
#include "../kernel/vga.h"

void pkg_init(void) {
    vga_puts("Package manager initialized (placeholder).\n");
}

int pkg_install(const char* package_name) {
    vga_puts("Installing package: ");
    vga_puts(package_name);
    vga_puts(" (placeholder)\n");
    return 0;
}

int pkg_uninstall(const char* package_name) {
    vga_puts("Uninstalling package: ");
    vga_puts(package_name);
    vga_puts(" (placeholder)\n");
    return 0;
}

int pkg_list(void) {
    vga_puts("Listing installed packages (placeholder).\n");
    return 0;
}


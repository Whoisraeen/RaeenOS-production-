#ifndef PKG_H
#define PKG_H

#include <stdint.h>

// Package metadata structure
typedef struct {
    char name[64];
    char version[32];
    char description[256];
    // Add more fields as needed (e.g., dependencies, install scripts)
} package_metadata_t;

// Initialize the package manager
void pkg_init(void);

// Install a package
int pkg_install(const char* package_name);

// Uninstall a package
int pkg_uninstall(const char* package_name);

// List installed packages
int pkg_list(void);

#endif // PKG_H

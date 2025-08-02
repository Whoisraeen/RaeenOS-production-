#ifndef RAMFS_H
#define RAMFS_H

#include <stdint.h>
#include <stddef.h>
#include "vfs.h"

// Initialize RAMFS
void ramfs_init(void);

// Mount RAMFS at a given path
int ramfs_mount(const char* path);

#endif // RAMFS_H
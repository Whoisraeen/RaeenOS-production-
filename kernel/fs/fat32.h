#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include "vfs.h"

// Initialize the FAT32 driver
void fat32_init(void);

// Mount a FAT32 partition
int fat32_mount(const char* device_path, const char* mount_point);

#endif // FAT32_H

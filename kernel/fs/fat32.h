#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include "vfs.h"

// Initialize the FAT32 driver
void fat32_init(void);

// Mount a FAT32 partition
int fat32_mount(const char* device_path, const char* mount_point);

// Read directory entries from a FAT32 cluster
int fat32_read_dir(uint32_t cluster, vfs_dirent_t* entries, uint32_t max_entries);

// Read data from a FAT32 file
int fat32_read_file(uint32_t start_cluster, uint32_t offset, uint32_t size, uint8_t* buffer);

#endif // FAT32_H

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

// Write data to a FAT32 file
int fat32_write_file(uint32_t start_cluster, uint32_t offset, uint32_t size, const uint8_t* buffer);

// Create a new file in a FAT32 directory
int fat32_create_file(uint32_t parent_cluster, const char* filename, uint32_t* new_cluster);

// Create a new directory in a FAT32 directory
int fat32_create_dir(uint32_t parent_cluster, const char* dirname, uint32_t* new_cluster);

// Delete a file or directory from a FAT32 directory
int fat32_delete_entry(uint32_t parent_cluster, const char* name);

// Placeholder for FAT32 journaling
void fat32_journal_start(void);
void fat32_journal_commit(void);
void fat32_journal_rollback(void);

#endif // FAT32_H

#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include "../vfs.h"

// FAT32 boot sector
typedef struct {
    uint8_t jmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_short;
    uint8_t media_type;
    uint16_t sectors_per_fat_short;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
    uint32_t sectors_per_fat_long;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t reserved2;
    uint8_t boot_sig;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
    // Journaling fields (placeholder)
    uint32_t journal_start_sector;
    uint32_t journal_size_sectors;
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 specific data to be stored in vfs_node_t->ptr
typedef struct {
    uint8_t drive_number;
    fat32_boot_sector_t boot_sector;
    uint32_t fat_start_sector;
    uint32_t data_start_sector;
    uint32_t root_dir_first_cluster;
} fat32_fs_data_t;

// FAT32 directory entry
typedef struct {
    uint8_t name[11];
    uint8_t attr;
    uint8_t ntres;
    uint8_t ctime_tenth;
    uint16_t ctime;
    uint16_t cdate;
    uint16_t adate;
    uint16_t hi_cluster;
    uint16_t mtime;
    uint16_t mdate;
    uint16_t lo_cluster;
    uint32_t size;
} __attribute__((packed)) fat32_dir_entry_t;

// Mount a FAT32 filesystem
vfs_node_t* fat32_mount(uint8_t* device);
vfs_node_t* fat32_readdir(vfs_node_t* node, uint32_t index);
vfs_node_t* fat32_finddir(vfs_node_t* node, const char* name);
void fat32_open(vfs_node_t* node, uint32_t flags);

#endif // FAT32_H

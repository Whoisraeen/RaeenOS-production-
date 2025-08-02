#include "fat32.h"
#include "../vga.h"
#include "../memory.h"
#include "../libs/libc/include/string.h"
#include "../../drivers/ata/ata.h"

// FAT32 Boot Sector Structure (simplified)
typedef struct {
    uint8_t  jmp[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // FAT32 specific fields
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved_0[12];
    uint8_t  drive_number;
    uint8_t  reserved_1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 Directory Entry (simplified)
typedef struct {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t fat_cluster_high;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t fat_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

static fat32_boot_sector_t boot_sector;
static uint32_t fat_start_sector;
static uint32_t cluster_start_sector;

void fat32_init(void) {
    debug_print("FAT32 filesystem initialized (placeholder).\n");
}

int fat32_mount(const char* device_path, const char* mount_point) {
    debug_print("FAT32: Attempting to mount ");
    debug_print(device_path);
    debug_print(" at ");
    debug_print(mount_point);
    debug_print("\n");

    // Simulate reading boot sector from device (drive 0 for now)
    uint8_t sector_buffer[512];
    if (ata_read_sectors(0, 0, 1, sector_buffer) != 0) {
        debug_print("FAT32: Failed to read boot sector.\n");
        return -1;
    }

    memcpy(&boot_sector, sector_buffer, sizeof(fat32_boot_sector_t));

    if (boot_sector.boot_signature != 0xAA55) {
        debug_print("FAT32: Invalid boot signature.\n");
        return -1;
    }

    fat_start_sector = boot_sector.reserved_sector_count;
    cluster_start_sector = fat_start_sector + (boot_sector.num_fats * boot_sector.fat_size_32);

    debug_print("FAT32: Mounted successfully. Root cluster: ");
    vga_put_hex(boot_sector.root_cluster);
    debug_print("\n");

    // Register with VFS (placeholder)
    // vfs_mount(mount_point, &fat32_vfs_ops); // Need to define fat32_vfs_ops

    return 0;
}

// Helper to get sector for a given cluster
static uint32_t fat32_cluster_to_sector(uint32_t cluster) {
    return cluster_start_sector + (cluster - 2) * boot_sector.sectors_per_cluster;
}

int fat32_read_dir(uint32_t cluster, vfs_dirent_t* entries, uint32_t max_entries) {
    debug_print("FAT32: Reading directory from cluster ");
    vga_put_hex(cluster);
    debug_print("\n");

    uint32_t sector = fat32_cluster_to_sector(cluster);
    uint8_t sector_buffer[512];
    uint32_t entry_count = 0;

    // Read first sector of the cluster
    if (ata_read_sectors(0, sector, 1, sector_buffer) != 0) {
        debug_print("FAT32: Failed to read directory sector.\n");
        return -1;
    }

    // Iterate through directory entries in the sector
    for (int i = 0; i < 512 / sizeof(fat32_dir_entry_t); i++) {
        fat32_dir_entry_t* dir_entry = (fat32_dir_entry_t*)(sector_buffer + i * sizeof(fat32_dir_entry_t));

        if (dir_entry->filename[0] == 0x00) break; // End of directory
        if (dir_entry->filename[0] == 0xE5) continue; // Deleted entry

        if (entry_count >= max_entries) break;

        // Populate vfs_dirent_t
        memset(entries[entry_count].name, 0, sizeof(entries[entry_count].name));
        // Copy filename (8 chars) and extension (3 chars)
        memcpy(entries[entry_count].name, dir_entry->filename, 8);
        if (dir_entry->ext[0] != ' ') {
            entries[entry_count].name[8] = '.';
            memcpy(entries[entry_count].name + 9, dir_entry->ext, 3);
        }
        // TODO: Convert to proper null-terminated string

        entries[entry_count].inode_num = (dir_entry->fat_cluster_high << 16) | dir_entry->fat_cluster_low;
        entries[entry_count].type = (dir_entry->attributes & 0x10) ? VFS_DIRECTORY : VFS_FILE; // 0x10 is directory attribute
        entry_count++;
    }

    return entry_count;
}

int fat32_read_file(uint32_t start_cluster, uint32_t offset, uint32_t size, uint8_t* buffer) {
    debug_print("FAT32: Reading file from cluster ");
    vga_put_hex(start_cluster);
    debug_print(" offset ");
    vga_put_dec(offset);
    debug_print(" size ");
    vga_put_dec(size);
    debug_print("\n");

    uint32_t current_cluster = start_cluster;
    uint32_t bytes_read = 0;
    uint8_t sector_buffer[512];

    // For simplicity, only read from the first cluster for now
    // A full implementation would traverse the FAT table to find all clusters.
    uint32_t sector = fat32_cluster_to_sector(current_cluster);
    if (ata_read_sectors(0, sector, 1, sector_buffer) != 0) {
        debug_print("FAT32: Failed to read file sector.\n");
        return -1;
    }

    // Copy data from the sector buffer
    uint32_t copy_size = (size > 512 - offset) ? (512 - offset) : size;
    memcpy(buffer, sector_buffer + offset, copy_size);
    bytes_read += copy_size;

    return bytes_read;
}

int fat32_write_file(uint32_t start_cluster, uint32_t offset, uint32_t size, const uint8_t* buffer) {
    debug_print("FAT32: Writing file to cluster ");
    vga_put_hex(start_cluster);
    debug_print(" offset ");
    vga_put_dec(offset);
    debug_print(" size ");
    vga_put_dec(size);
    debug_print(" (simulated).\n");
    // In a real implementation, this would write data to the file's clusters.
    return size;
}

int fat32_create_file(uint32_t parent_cluster, const char* filename, uint32_t* new_cluster) {
    debug_print("FAT32: Creating file ");
    debug_print(filename);
    debug_print(" in cluster ");
    vga_put_hex(parent_cluster);
    debug_print(" (simulated).\n");
    // In a real implementation, this would find a free cluster, update FAT, and create directory entry.
    if (new_cluster) *new_cluster = 0; // Dummy cluster
    return 0;
}

int fat32_create_dir(uint32_t parent_cluster, const char* dirname, uint32_t* new_cluster) {
    debug_print("FAT32: Creating directory ");
    debug_print(dirname);
    debug_print(" in cluster ");
    vga_put_hex(parent_cluster);
    debug_print(" (simulated).\n");
    // In a real implementation, this would find a free cluster, update FAT, and create directory entry.
    if (new_cluster) *new_cluster = 0; // Dummy cluster
    return 0;
}

int fat32_delete_entry(uint32_t parent_cluster, const char* name) {
    debug_print("FAT32: Deleting ");
    debug_print(name);
    debug_print(" from cluster ");
    vga_put_hex(parent_cluster);
    debug_print(" (simulated).\n");
    // In a real implementation, this would mark directory entry as deleted and free clusters.
    return 0;
}

// Placeholder for FAT32 journaling
void fat32_journal_start(void) {
    debug_print("FAT32 Journal: Transaction started (simulated).\n");
}

void fat32_journal_commit(void) {
    debug_print("FAT32 Journal: Transaction committed (simulated).\n");
}

void fat32_journal_rollback(void) {
    debug_print("FAT32 Journal: Transaction rolled back (simulated).\n");
}

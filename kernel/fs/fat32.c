#include "fat32.h"
#include "../vga.h"
#include "../memory.h"
#include "../string.h"
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

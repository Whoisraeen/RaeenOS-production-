#include "fat32.h"
#include "../vfs.h"
#include "../../memory.h"
#include "../../../drivers/ata/ata.h"
#include "../../string.h" // For memcpy, memset, etc.

// Read a sector from the device
static int read_sector(uint8_t drive_number, uint32_t sector, uint8_t* buf) {
    return ata_read_sectors(drive_number, sector, 1, (uint16_t*)buf);
}

// Convert 8.3 FAT filename to null-terminated string
static void fat_name_to_string(const uint8_t* fat_name, char* out_name) {
    int i, j;
    for (i = 0; i < 8 && fat_name[i] != ' '; i++) {
        out_name[i] = fat_name[i];
    }
    if (fat_name[8] != ' ') { // Has extension
        out_name[i++] = '.';
        for (j = 8; j < 11 && fat_name[j] != ' '; j++) {
            out_name[i++] = fat_name[j];
        }
    }
    out_name[i] = '\0';
}

// Get the first sector of a cluster
static uint32_t fat32_cluster_to_sector(fat32_fs_data_t* fs_data, uint32_t cluster) {
    // Data region starts after boot sector and FATs
    return fs_data->data_start_sector + (cluster - 2) * fs_data->boot_sector.sectors_per_cluster;
}

// Read a directory entry
struct dirent* fat32_readdir(vfs_node_t* node, uint32_t index) {
    if (!(node->flags & VFS_DIRECTORY)) {
        return NULL;
    }

    fat32_fs_data_t* fs_data = (fat32_fs_data_t*)node->fs_private_data;
    if (!fs_data) {
        return NULL;
    }

    // For simplicity, assume root directory for now
    // In a real implementation, this would traverse clusters for subdirectories
    uint32_t current_cluster = fs_data->root_dir_first_cluster;
    uint8_t sector_buffer[512];
    static struct dirent dirent_entry; // Static to return pointer

    uint32_t entries_per_sector = fs_data->boot_sector.bytes_per_sector / sizeof(fat32_dir_entry_t);
    uint32_t entry_sector_offset = index / entries_per_sector;
    uint32_t entry_in_sector_index = index % entries_per_sector;

    // Calculate the sector containing the directory entry
    uint32_t dir_sector = fat32_cluster_to_sector(fs_data, current_cluster) + entry_sector_offset;

    if (read_sector(fs_data->drive_number, dir_sector, sector_buffer) != 0) {
        return NULL;
    }

    fat32_dir_entry_t* dir_entry = (fat32_dir_entry_t*)(sector_buffer + entry_in_sector_index * sizeof(fat32_dir_entry_t));

    if (dir_entry->name[0] == 0x00) { // End of directory
        return NULL;
    }
    if (dir_entry->name[0] == 0xE5) { // Deleted entry
        return NULL;
    }

    fat_name_to_string(dir_entry->name, dirent_entry.name);
    dirent_entry.inode_num = 0; // FAT doesn't use inodes in the same way

    return &dirent_entry;
}

vfs_node_t* fat32_finddir(vfs_node_t* node, const char* name) {
    if (!(node->flags & VFS_DIRECTORY)) {
        return NULL;
    }

    fat32_fs_data_t* fs_data = (fat32_fs_data_t*)node->fs_private_data;
    if (!fs_data) {
        return NULL;
    }

    uint32_t current_cluster = fs_data->root_dir_first_cluster;
    uint8_t sector_buffer[512];
    char entry_name[VFS_FILENAME_MAX];

    uint32_t entries_per_sector = fs_data->boot_sector.bytes_per_sector / sizeof(fat32_dir_entry_t);
    uint32_t sectors_per_cluster = fs_data->boot_sector.sectors_per_cluster;

    // Iterate through clusters of the directory
    // For simplicity, only handling single cluster directories for now
    // A real implementation would follow the FAT chain
    for (uint32_t i = 0; i < sectors_per_cluster; i++) {
        uint32_t dir_sector = fat32_cluster_to_sector(fs_data, current_cluster) + i;
        if (read_sector(fs_data->drive_number, dir_sector, sector_buffer) != 0) {
            return NULL;
        }

        for (uint32_t j = 0; j < entries_per_sector; j++) {
            fat32_dir_entry_t* dir_entry = (fat32_dir_entry_t*)(sector_buffer + j * sizeof(fat32_dir_entry_t));

            if (dir_entry->name[0] == 0x00) { // End of directory
                return NULL;
            }
            if (dir_entry->name[0] == 0xE5) { // Deleted entry
                continue;
            }

            fat_name_to_string(dir_entry->name, entry_name);
            if (strcmp(entry_name, name) == 0) {
                // Found the entry, create a vfs_node for it
                vfs_node_t* new_node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
                if (!new_node) return NULL;

                strcpy(new_node->name, entry_name);
                new_node->inode = 0; // Not used in FAT
                new_node->length = dir_entry->size;
                new_node->permissions = 0; // TODO: Map FAT attributes to VFS permissions

                if (dir_entry->attr & 0x10) { // Directory attribute
                    new_node->flags = VFS_DIRECTORY;
                    new_node->readdir = fat32_readdir; // Assign readdir for subdirectories
                    new_node->finddir = fat32_finddir; // Assign finddir for subdirectories
                } else {
                    new_node->flags = VFS_FILE;
                    new_node->read = NULL; // TODO: Implement fat32_read
                    new_node->write = NULL; // TODO: Implement fat32_write
                    new_node->open = fat32_open; // Assign the new open function
                }
                new_node->close = NULL; // TODO: Implement fat32_close
                new_node->fs_private_data = fs_data; // Link to filesystem data
                new_node->pipe = NULL;
                new_node->mounted_at = NULL;

                return new_node;
            }
        }
    }
    return NULL; // Not found
}

// Placeholder for fat32_open
static void fat32_open(vfs_node_t* node, uint32_t flags) {
    // For now, just acknowledge the open call and flags.
    // In a real implementation, this would set up internal file pointers,
    // check permissions based on flags, etc.
    (void)node; // Suppress unused parameter warning
    (void)flags; // Suppress unused parameter warning
}

vfs_node_t* fat32_mount(uint8_t* device) {
    fat32_fs_data_t* fs_data = (fat32_fs_data_t*)kmalloc(sizeof(fat32_fs_data_t));
    if (!fs_data) {
        return NULL;
    }

    fs_data->drive_number = (uint8_t)(uintptr_t)device;

    if (read_sector(fs_data->drive_number, 0, (uint8_t*)&fs_data->boot_sector) != 0) {
        kfree(fs_data);
        return NULL;
    }

    // Calculate FAT and data start sectors
    fs_data->fat_start_sector = fs_data->boot_sector.reserved_sectors;
    fs_data->data_start_sector = fs_data->fat_start_sector + (fs_data->boot_sector.num_fats * fs_data->boot_sector.sectors_per_fat_long);
    fs_data->root_dir_first_cluster = fs_data->boot_sector.root_cluster;

    // Create the root node
    vfs_node_t* root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!root) {
        kfree(fs_data);
        return NULL;
    }
    root->name[0] = '/';
    root->name[1] = '\0';
    root->flags = VFS_DIRECTORY;
    root->read = NULL;
    root->write = NULL;
    root->open = NULL;
    root->close = NULL;
    root->readdir = fat32_readdir; // Assign readdir for the root
    root->finddir = fat32_finddir; // Assign finddir for the root
    root->create = NULL; // Not implemented yet
    root->inode = 0;
    root->length = 0;
    root->permissions = 0;
    root->pipe = NULL;
    root->mounted_at = NULL;
    root->fs_private_data = fs_data; // Link to filesystem data

    return root;
}

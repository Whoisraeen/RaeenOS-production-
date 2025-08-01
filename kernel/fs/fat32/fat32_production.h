/**
 * @file fat32_production.h
 * @brief RaeenOS Production FAT32 Filesystem Implementation
 * 
 * Enterprise-grade FAT32 with:
 * - Full VFAT long filename support with Unicode
 * - Journaling for crash consistency
 * - Advanced caching and performance optimization
 * - Write support with proper cluster allocation
 * - Directory tree traversal and management
 * - File attribute and permission mapping
 * - Fragmentation analysis and defragmentation
 * - Bad sector handling and recovery
 * 
 * Version: 2.0 - Production Ready
 * Compliance: Microsoft FAT32 specification
 * Performance Target: >500MB/s sequential, >50k IOPS random
 */

#ifndef FAT32_PRODUCTION_H
#define FAT32_PRODUCTION_H

#include "include/types.h"
// Using types.h for kernel build
#include "../vfs.h"
#include "../buffer_cache.h"
#include "../vfs_events.h"
#include "../../include/types.h"
#include "../../include/sync.h"

#ifdef __cplusplus
extern "C" {
#endif

// FAT32 constants
#define FAT32_SIGNATURE        0xAA55
#define FAT32_FSINFO_SIGNATURE 0x41615252
#define FAT32_FSINFO_SIGNATURE2 0x61417272
#define FAT32_CLUSTER_EOF      0x0FFFFFFF
#define FAT32_CLUSTER_BAD      0x0FFFFFF7
#define FAT32_CLUSTER_FREE     0x00000000
#define FAT32_CLUSTER_RESERVED 0x0FFFFFF0

// Directory entry attributes
#define FAT32_ATTR_READ_ONLY   0x01
#define FAT32_ATTR_HIDDEN      0x02
#define FAT32_ATTR_SYSTEM      0x04
#define FAT32_ATTR_VOLUME_ID   0x08
#define FAT32_ATTR_DIRECTORY   0x10
#define FAT32_ATTR_ARCHIVE     0x20
#define FAT32_ATTR_LONG_NAME   0x0F

// Long filename entry constants
#define FAT32_LFN_LAST         0x40
#define FAT32_LFN_DELETED      0xE5
#define FAT32_LFN_MAX_ENTRIES  20
#define FAT32_LFN_CHARS_PER_ENTRY 13

// Performance and cache settings
#define FAT32_CACHE_CLUSTERS   1024
#define FAT32_CACHE_DIRENTS    512
#define FAT32_READAHEAD_CLUSTERS 32
#define FAT32_MAX_CLUSTER_CHAIN 65536

// Forward declarations
struct fat32_mount;
struct fat32_file;
struct fat32_directory;

// FAT32 Boot Sector (Enhanced)
typedef struct {
    uint8_t jmp[3];                    // Jump instruction
    uint8_t oem_name[8];               // OEM name
    uint16_t bytes_per_sector;         // Bytes per sector
    uint8_t sectors_per_cluster;       // Sectors per cluster
    uint16_t reserved_sectors;         // Reserved sectors
    uint8_t num_fats;                  // Number of FATs
    uint16_t root_entries;             // Root directory entries (0 for FAT32)
    uint16_t total_sectors_short;      // Total sectors (0 for FAT32)
    uint8_t media_type;                // Media descriptor
    uint16_t sectors_per_fat_short;    // Sectors per FAT (0 for FAT32)
    uint16_t sectors_per_track;        // Sectors per track
    uint16_t num_heads;                // Number of heads
    uint32_t hidden_sectors;           // Hidden sectors
    uint32_t total_sectors_long;       // Total sectors
    
    // FAT32 specific fields
    uint32_t sectors_per_fat_long;     // Sectors per FAT
    uint16_t flags;                    // Flags
    uint16_t version;                  // Version
    uint32_t root_cluster;             // Root directory cluster
    uint16_t fsinfo_sector;            // FSInfo sector
    uint16_t backup_boot_sector;       // Backup boot sector
    uint8_t reserved[12];              // Reserved
    uint8_t drive_num;                 // Drive number
    uint8_t reserved2;                 // Reserved
    uint8_t boot_sig;                  // Boot signature
    uint32_t volume_id;                // Volume ID
    uint8_t volume_label[11];          // Volume label
    uint8_t fs_type[8];                // Filesystem type
    
    // Boot code
    uint8_t boot_code[420];            // Boot code
    uint16_t signature;                // Boot sector signature (0xAA55)
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 FSInfo Sector
typedef struct {
    uint32_t lead_sig;                 // Lead signature (0x41615252)
    uint8_t reserved1[480];            // Reserved
    uint32_t struct_sig;               // Structure signature (0x61417272)
    uint32_t free_count;               // Free cluster count
    uint32_t next_free;                // Next free cluster
    uint8_t reserved2[12];             // Reserved
    uint32_t trail_sig;                // Trail signature (0xAA550000)
} __attribute__((packed)) fat32_fsinfo_t;

// Standard directory entry
typedef struct {
    uint8_t name[11];                  // 8.3 filename
    uint8_t attr;                      // File attributes
    uint8_t ntres;                     // NT reserved
    uint8_t ctime_tenth;               // Creation time (tenth of second)
    uint16_t ctime;                    // Creation time
    uint16_t cdate;                    // Creation date
    uint16_t adate;                    // Access date
    uint16_t hi_cluster;               // High 16 bits of cluster
    uint16_t mtime;                    // Modification time
    uint16_t mdate;                    // Modification date
    uint16_t lo_cluster;               // Low 16 bits of cluster
    uint32_t size;                     // File size
} __attribute__((packed)) fat32_dir_entry_t;

// Long filename directory entry
typedef struct {
    uint8_t order;                     // Order of entry
    uint16_t name1[5];                 // Characters 1-5
    uint8_t attr;                      // Attributes (always 0x0F)
    uint8_t type;                      // Type (always 0)
    uint8_t checksum;                  // Checksum of short name
    uint16_t name2[6];                 // Characters 6-11
    uint16_t cluster;                  // First cluster (always 0)
    uint16_t name3[2];                 // Characters 12-13
} __attribute__((packed)) fat32_lfn_entry_t;

// Cluster chain cache entry
typedef struct fat32_cluster_cache {
    uint32_t cluster;                  // Cluster number
    uint32_t next;                     // Next cluster in chain
    bool dirty;                        // Needs to be written
    uint64_t last_access;              // Last access time
    struct fat32_cluster_cache* hash_next;
    struct fat32_cluster_cache* lru_next;
    struct fat32_cluster_cache* lru_prev;
} fat32_cluster_cache_t;

// Directory entry cache
typedef struct fat32_dirent_cache {
    char name[VFS_FILENAME_MAX];       // Full filename
    uint32_t cluster;                  // First cluster
    uint32_t size;                     // File size
    uint8_t attr;                      // Attributes
    uint64_t mtime;                    // Modification time
    uint32_t parent_cluster;           // Parent directory cluster
    bool valid;                        // Cache entry is valid
    uint64_t last_access;              // Last access time
    struct fat32_dirent_cache* hash_next;
    struct fat32_dirent_cache* lru_next;
    struct fat32_dirent_cache* lru_prev;
} fat32_dirent_cache_t;

// FAT32 mount structure
typedef struct fat32_mount {
    // Filesystem information
    fat32_boot_sector_t boot_sector;
    fat32_fsinfo_t fsinfo;
    
    // Calculated parameters
    uint32_t fat_start_sector;         // First FAT sector
    uint32_t data_start_sector;        // First data sector
    uint32_t total_clusters;           // Total clusters
    uint32_t cluster_size;             // Cluster size in bytes
    uint32_t entries_per_cluster;      // Directory entries per cluster
    
    // Device information
    uint64_t device_id;                // Device identifier
    uint32_t sector_size;              // Sector size
    
    // Caching
    fat32_cluster_cache_t* cluster_cache[FAT32_CACHE_CLUSTERS];
    fat32_dirent_cache_t* dirent_cache[FAT32_CACHE_DIRENTS];
    fat32_cluster_cache_t* cluster_lru_head;
    fat32_cluster_cache_t* cluster_lru_tail;
    fat32_dirent_cache_t* dirent_lru_head;
    fat32_dirent_cache_t* dirent_lru_tail;
    
    // Free space management
    uint32_t free_clusters;            // Number of free clusters
    uint32_t next_free_cluster;        // Next free cluster hint
    uint32_t* free_cluster_bitmap;     // Free cluster bitmap
    
    // Performance statistics
    uint64_t reads;                    // Read operations
    uint64_t writes;                   // Write operations
    uint64_t cache_hits;               // Cache hits
    uint64_t cache_misses;             // Cache misses
    uint64_t cluster_allocations;      // Cluster allocations
    uint64_t cluster_deallocations;    // Cluster deallocations
    
    // Synchronization
    rwlock_t mount_lock;               // Mount-wide lock
    spinlock_t cache_lock;             // Cache lock
    spinlock_t fat_lock;               // FAT access lock
    
    // Fragmentation analysis
    uint32_t fragmented_files;         // Number of fragmented files
    uint32_t max_contiguous_free;      // Largest contiguous free space
    uint64_t last_defrag_time;         // Last defragmentation time
    
    // Error handling
    uint32_t bad_sectors;              // Number of bad sectors
    uint32_t* bad_cluster_list;        // List of bad clusters
    size_t bad_cluster_count;          // Number of bad clusters
    
    // Journaling (for crash consistency)
    bool journaling_enabled;           // Journal enabled
    uint32_t journal_cluster;          // Journal cluster
    uint32_t journal_size;             // Journal size in clusters
    void* journal_buffer;              // Journal buffer
    
    // Mount options
    bool read_only;                    // Read-only mount
    bool case_sensitive;               // Case-sensitive names
    bool force_lowercase;              // Force lowercase names
    uint32_t umask;                    // Default umask
    
    // Reference counting
    atomic_t ref_count;                // Reference count
} fat32_mount_t;

// FAT32 file handle
typedef struct fat32_file {
    vfs_file_t* vfs_file;              // VFS file structure
    fat32_mount_t* mount;              // Mount point
    
    uint32_t first_cluster;            // First cluster of file
    uint32_t current_cluster;          // Current cluster
    uint32_t cluster_offset;           // Offset within current cluster
    uint64_t file_position;            // Current file position
    
    // Cluster chain cache
    uint32_t* cluster_chain;           // Cached cluster chain
    size_t cluster_chain_size;         // Size of cluster chain
    bool cluster_chain_valid;          // Chain is valid
    
    // Performance optimization
    uint32_t last_accessed_cluster;    // Last accessed cluster
    uint32_t last_cluster_index;       // Index of last accessed cluster
    
    spinlock_t lock;                   // File lock
} fat32_file_t;

// FAT32 directory handle
typedef struct fat32_directory {
    vfs_file_t* vfs_file;              // VFS file structure
    fat32_mount_t* mount;              // Mount point
    
    uint32_t first_cluster;            // First cluster of directory
    uint32_t current_cluster;          // Current cluster
    uint32_t entry_index;              // Current entry index
    
    // Cached entries
    fat32_dir_entry_t* entries;        // Cached directory entries
    size_t entry_count;                // Number of cached entries
    bool entries_valid;                // Cache is valid
    
    spinlock_t lock;                   // Directory lock
} fat32_directory_t;

// FAT32 filesystem operations
extern vfs_fs_operations_t fat32_fs_ops;

// Core FAT32 functions

/**
 * Initialize FAT32 filesystem driver
 */
int fat32_init(void);

/**
 * Shutdown FAT32 filesystem driver
 */
void fat32_shutdown(void);

/**
 * Mount FAT32 filesystem
 */
vfs_superblock_t* fat32_mount_fs(const char* device, uint32_t flags, const void* data);

/**
 * Unmount FAT32 filesystem
 */
void fat32_unmount_fs(vfs_superblock_t* sb);

// Cluster management

/**
 * Read cluster from disk
 */
int fat32_read_cluster(fat32_mount_t* mount, uint32_t cluster, void* buffer);

/**
 * Write cluster to disk
 */
int fat32_write_cluster(fat32_mount_t* mount, uint32_t cluster, const void* buffer);

/**
 * Allocate new cluster
 */
uint32_t fat32_alloc_cluster(fat32_mount_t* mount, uint32_t prev_cluster);

/**
 * Free cluster
 */
int fat32_free_cluster(fat32_mount_t* mount, uint32_t cluster);

/**
 * Get next cluster in chain
 */
uint32_t fat32_get_next_cluster(fat32_mount_t* mount, uint32_t cluster);

/**
 * Set next cluster in chain
 */
int fat32_set_next_cluster(fat32_mount_t* mount, uint32_t cluster, uint32_t next);

/**
 * Get cluster chain
 */
int fat32_get_cluster_chain(fat32_mount_t* mount, uint32_t first_cluster, 
                           uint32_t** chain, size_t* count);

/**
 * Free cluster chain
 */
int fat32_free_cluster_chain(fat32_mount_t* mount, uint32_t first_cluster);

// Directory operations

/**
 * Read directory entries
 */
int fat32_read_directory(fat32_mount_t* mount, uint32_t cluster, 
                        fat32_dir_entry_t** entries, size_t* count);

/**
 * Find directory entry
 */
int fat32_find_dirent(fat32_mount_t* mount, uint32_t dir_cluster, 
                     const char* name, fat32_dir_entry_t* entry, uint32_t* entry_cluster);

/**
 * Create directory entry
 */
int fat32_create_dirent(fat32_mount_t* mount, uint32_t dir_cluster, 
                       const char* name, uint8_t attr, uint32_t first_cluster, uint32_t size);

/**
 * Delete directory entry
 */
int fat32_delete_dirent(fat32_mount_t* mount, uint32_t dir_cluster, const char* name);

/**
 * Update directory entry
 */
int fat32_update_dirent(fat32_mount_t* mount, uint32_t dir_cluster, 
                       const char* name, const fat32_dir_entry_t* entry);

// File operations

/**
 * Open file
 */
int fat32_file_open(vfs_inode_t* inode, vfs_file_t* file);

/**
 * Close file
 */
int fat32_file_close(vfs_file_t* file);

/**
 * Read from file
 */
ssize_t fat32_file_read(vfs_file_t* file, void* buffer, size_t count, off_t* offset);

/**
 * Write to file
 */
ssize_t fat32_file_write(vfs_file_t* file, const void* buffer, size_t count, off_t* offset);

/**
 * Seek in file
 */
off_t fat32_file_seek(vfs_file_t* file, off_t offset, int whence);

/**
 * Truncate file
 */
int fat32_file_truncate(vfs_file_t* file, off_t length);

/**
 * Sync file
 */
int fat32_file_sync(vfs_file_t* file, int datasync);

// Long filename support

/**
 * Convert long filename to short filename
 */
int fat32_long_to_short_name(const char* long_name, uint8_t* short_name);

/**
 * Generate LFN entries
 */
int fat32_generate_lfn_entries(const char* long_name, uint8_t checksum, 
                              fat32_lfn_entry_t* entries, size_t* count);

/**
 * Read long filename
 */
int fat32_read_long_filename(fat32_mount_t* mount, uint32_t dir_cluster, 
                            uint32_t entry_index, char* name, size_t name_size);

/**
 * Calculate LFN checksum
 */
uint8_t fat32_lfn_checksum(const uint8_t* short_name);

// Cache management

/**
 * Initialize cluster cache
 */
int fat32_init_cluster_cache(fat32_mount_t* mount);

/**
 * Cleanup cluster cache
 */
void fat32_cleanup_cluster_cache(fat32_mount_t* mount);

/**
 * Get cluster from cache
 */
uint32_t fat32_cache_get_cluster(fat32_mount_t* mount, uint32_t cluster);

/**
 * Set cluster in cache
 */
void fat32_cache_set_cluster(fat32_mount_t* mount, uint32_t cluster, uint32_t next);

/**
 * Flush cluster cache
 */
int fat32_flush_cluster_cache(fat32_mount_t* mount);

/**
 * Initialize directory entry cache
 */
int fat32_init_dirent_cache(fat32_mount_t* mount);

/**
 * Cleanup directory entry cache
 */
void fat32_cleanup_dirent_cache(fat32_mount_t* mount);

// Utility functions

/**
 * Convert FAT32 time to Unix timestamp
 */
uint64_t fat32_time_to_unix(uint16_t date, uint16_t time, uint8_t tenth);

/**
 * Convert Unix timestamp to FAT32 time
 */
void fat32_unix_to_time(uint64_t timestamp, uint16_t* date, uint16_t* time, uint8_t* tenth);

/**
 * Check if cluster is valid
 */
bool fat32_is_cluster_valid(fat32_mount_t* mount, uint32_t cluster);

/**
 * Check if cluster is EOF
 */
bool fat32_is_cluster_eof(uint32_t cluster);

/**
 * Check if cluster is bad
 */
bool fat32_is_cluster_bad(uint32_t cluster);

/**
 * Validate FAT32 boot sector
 */
bool fat32_validate_boot_sector(const fat32_boot_sector_t* boot_sector);

// Performance and analysis

/**
 * Analyze filesystem fragmentation
 */
int fat32_analyze_fragmentation(fat32_mount_t* mount);

/**
 * Defragment filesystem
 */
int fat32_defragment(fat32_mount_t* mount);

/**
 * Get filesystem statistics
 */
int fat32_get_stats(fat32_mount_t* mount, struct statfs* stats);

/**
 * Check filesystem consistency
 */
int fat32_check_consistency(fat32_mount_t* mount, bool repair);

// Error recovery

/**
 * Mark cluster as bad
 */
int fat32_mark_bad_cluster(fat32_mount_t* mount, uint32_t cluster);

/**
 * Recover from bad sectors
 */
int fat32_recover_bad_sectors(fat32_mount_t* mount);

/**
 * Rebuild FAT from directory structure
 */
int fat32_rebuild_fat(fat32_mount_t* mount);

// Journaling support

/**
 * Initialize journal
 */
int fat32_init_journal(fat32_mount_t* mount);

/**
 * Write journal entry
 */
int fat32_journal_write(fat32_mount_t* mount, const void* data, size_t size);

/**
 * Commit journal
 */
int fat32_journal_commit(fat32_mount_t* mount);

/**
 * Replay journal
 */
int fat32_journal_replay(fat32_mount_t* mount);

// Error codes
#define FAT32_SUCCESS           0
#define FAT32_ERR_NO_MEMORY    -6001
#define FAT32_ERR_IO_ERROR     -6002
#define FAT32_ERR_INVALID_ARG  -6003
#define FAT32_ERR_NOT_FOUND    -6004
#define FAT32_ERR_EXISTS       -6005
#define FAT32_ERR_NO_SPACE     -6006
#define FAT32_ERR_READ_ONLY    -6007
#define FAT32_ERR_CORRUPTED    -6008
#define FAT32_ERR_BAD_CLUSTER  -6009
#define FAT32_ERR_INVALID_NAME -6010
#define FAT32_ERR_NOT_SUPPORTED -6011

#ifdef __cplusplus
}
#endif

#endif // FAT32_PRODUCTION_H
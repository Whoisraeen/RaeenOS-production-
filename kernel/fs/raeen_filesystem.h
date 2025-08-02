#pragma once

/**
 * RaeenOS High-Level File System API
 * 
 * A comprehensive, user-friendly file system interface that provides:
 * - Simple file operations (create, read, write, delete)
 * - Advanced file management (permissions, metadata, search)
 * - Cross-platform compatibility
 * - High performance with automatic optimization
 * - Integration with existing VFS production system
 * - Support for multiple file systems (FAT32, EXT4, NTFS, RaeenFS)
 */

#include "vfs_production.h"
#include "../include/types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// SIMPLIFIED FILE SYSTEM TYPES
// ============================================================================

typedef struct RaeenFile RaeenFile;
typedef struct RaeenDirectory RaeenDirectory;
typedef struct RaeenFileInfo RaeenFileInfo;
typedef struct RaeenFileSystemInfo RaeenFileSystemInfo;

// File access modes
typedef enum {
    RAEEN_FILE_READ      = 0x01,
    RAEEN_FILE_WRITE     = 0x02,
    RAEEN_FILE_APPEND    = 0x04,
    RAEEN_FILE_CREATE    = 0x08,
    RAEEN_FILE_TRUNCATE  = 0x10,
    RAEEN_FILE_BINARY    = 0x20,
    RAEEN_FILE_EXCLUSIVE = 0x40
} RaeenFileMode;

// File types
typedef enum {
    RAEEN_TYPE_UNKNOWN = 0,
    RAEEN_TYPE_FILE,
    RAEEN_TYPE_DIRECTORY,
    RAEEN_TYPE_SYMLINK,
    RAEEN_TYPE_DEVICE,
    RAEEN_TYPE_PIPE,
    RAEEN_TYPE_SOCKET
} RaeenFileType;

// File attributes
typedef enum {
    RAEEN_ATTR_READONLY   = 0x01,
    RAEEN_ATTR_HIDDEN     = 0x02,
    RAEEN_ATTR_SYSTEM     = 0x04,
    RAEEN_ATTR_ARCHIVE    = 0x08,
    RAEEN_ATTR_COMPRESSED = 0x10,
    RAEEN_ATTR_ENCRYPTED  = 0x20,
    RAEEN_ATTR_TEMPORARY  = 0x40,
    RAEEN_ATTR_SPARSE     = 0x80
} RaeenFileAttributes;

// File permissions (Unix-style)
typedef enum {
    RAEEN_PERM_READ_OWNER   = 0400,
    RAEEN_PERM_WRITE_OWNER  = 0200,
    RAEEN_PERM_EXEC_OWNER   = 0100,
    RAEEN_PERM_READ_GROUP   = 0040,
    RAEEN_PERM_WRITE_GROUP  = 0020,
    RAEEN_PERM_EXEC_GROUP   = 0010,
    RAEEN_PERM_READ_OTHER   = 0004,
    RAEEN_PERM_WRITE_OTHER  = 0002,
    RAEEN_PERM_EXEC_OTHER   = 0001
} RaeenFilePermissions;

// Seek origins
typedef enum {
    RAEEN_SEEK_BEGIN   = 0,
    RAEEN_SEEK_CURRENT = 1,
    RAEEN_SEEK_END     = 2
} RaeenSeekOrigin;

// File information structure
typedef struct RaeenFileInfo {
    char name[256];              // File name
    char full_path[1024];        // Full path
    RaeenFileType type;          // File type
    uint64_t size;               // File size in bytes
    uint64_t created_time;       // Creation time (Unix timestamp)
    uint64_t modified_time;      // Last modification time
    uint64_t accessed_time;      // Last access time
    uint32_t permissions;        // File permissions
    uint32_t attributes;         // File attributes
    uint32_t owner_id;           // Owner user ID
    uint32_t group_id;           // Group ID
    uint32_t device_id;          // Device ID (for device files)
    uint32_t link_count;         // Number of hard links
    char* symlink_target;        // Target path (for symlinks)
    uint64_t allocated_size;     // Actual disk space used
    bool is_compressed;          // File is compressed
    bool is_encrypted;           // File is encrypted
    char checksum[64];           // File integrity checksum
} RaeenFileInfo;

// Directory entry
typedef struct RaeenDirectoryEntry {
    char name[256];
    RaeenFileType type;
    uint64_t size;
    uint32_t permissions;
    uint64_t modified_time;
    bool is_hidden;
    struct RaeenDirectoryEntry* next;
} RaeenDirectoryEntry;

// File system information
typedef struct RaeenFileSystemInfo {
    char name[64];               // File system name (e.g., "ext4", "ntfs")
    char mount_point[256];       // Mount point path
    char device[256];            // Device path
    uint64_t total_space;        // Total space in bytes
    uint64_t free_space;         // Free space in bytes
    uint64_t available_space;    // Available space to user
    uint32_t block_size;         // File system block size
    uint64_t total_inodes;       // Total inodes
    uint64_t free_inodes;        // Free inodes
    bool is_readonly;            // Read-only file system
    bool supports_permissions;   // Supports Unix permissions
    bool supports_symlinks;      // Supports symbolic links
    bool supports_hardlinks;     // Supports hard links
    bool supports_xattrs;        // Supports extended attributes
    bool supports_compression;   // Supports compression
    bool supports_encryption;    // Supports encryption
} RaeenFileSystemInfo;

// File handle structure
struct RaeenFile {
    vfs_file_t* vfs_file;        // Underlying VFS file
    uint32_t mode;               // Open mode flags
    uint64_t position;           // Current file position
    bool is_open;                // File is open
    bool is_dirty;               // File has unsaved changes
    char path[1024];             // File path
    RaeenFileInfo info;          // Cached file information
};

// Directory handle structure  
struct RaeenDirectory {
    vfs_file_t* vfs_dir;         // Underlying VFS directory
    char path[1024];             // Directory path
    RaeenDirectoryEntry* entries; // Cached directory entries
    RaeenDirectoryEntry* current; // Current entry for iteration
    bool is_open;                // Directory is open
    int entry_count;             // Number of entries
};

// ============================================================================
// FILE OPERATIONS
// ============================================================================

// Basic file operations
RaeenFile* raeen_file_open(const char* path, RaeenFileMode mode);
int raeen_file_close(RaeenFile* file);
ssize_t raeen_file_read(RaeenFile* file, void* buffer, size_t size);
ssize_t raeen_file_write(RaeenFile* file, const void* buffer, size_t size);
ssize_t raeen_file_read_at(RaeenFile* file, void* buffer, size_t size, uint64_t offset);
ssize_t raeen_file_write_at(RaeenFile* file, const void* buffer, size_t size, uint64_t offset);
int raeen_file_flush(RaeenFile* file);
int raeen_file_sync(RaeenFile* file);

// File positioning
uint64_t raeen_file_tell(RaeenFile* file);
int raeen_file_seek(RaeenFile* file, int64_t offset, RaeenSeekOrigin origin);
int raeen_file_rewind(RaeenFile* file);
bool raeen_file_eof(RaeenFile* file);

// File management
int raeen_file_create(const char* path, uint32_t permissions);
int raeen_file_delete(const char* path);
int raeen_file_copy(const char* source, const char* destination);
int raeen_file_move(const char* source, const char* destination);
int raeen_file_rename(const char* old_path, const char* new_path);
int raeen_file_link(const char* target, const char* link_path);
int raeen_file_symlink(const char* target, const char* link_path);
bool raeen_file_exists(const char* path);

// File attributes and information
int raeen_file_get_info(const char* path, RaeenFileInfo* info);
int raeen_file_set_permissions(const char* path, uint32_t permissions);
int raeen_file_set_attributes(const char* path, uint32_t attributes);
int raeen_file_set_times(const char* path, uint64_t access_time, uint64_t modify_time);
int raeen_file_set_owner(const char* path, uint32_t owner_id, uint32_t group_id);
uint64_t raeen_file_get_size(const char* path);
bool raeen_file_is_directory(const char* path);
bool raeen_file_is_readable(const char* path);
bool raeen_file_is_writable(const char* path);
bool raeen_file_is_executable(const char* path);

// Advanced file operations
int raeen_file_truncate(const char* path, uint64_t size);
int raeen_file_allocate(const char* path, uint64_t size);
int raeen_file_compress(const char* path);
int raeen_file_decompress(const char* path);
int raeen_file_encrypt(const char* path, const char* key);
int raeen_file_decrypt(const char* path, const char* key);
int raeen_file_checksum(const char* path, char* checksum, size_t checksum_size);

// ============================================================================
// DIRECTORY OPERATIONS
// ============================================================================

// Basic directory operations
RaeenDirectory* raeen_directory_open(const char* path);
int raeen_directory_close(RaeenDirectory* dir);
RaeenDirectoryEntry* raeen_directory_read(RaeenDirectory* dir);
void raeen_directory_rewind(RaeenDirectory* dir);
int raeen_directory_create(const char* path, uint32_t permissions);
int raeen_directory_delete(const char* path);
bool raeen_directory_exists(const char* path);
bool raeen_directory_is_empty(const char* path);

// Directory traversal
int raeen_directory_list(const char* path, RaeenDirectoryEntry** entries, int* count);
int raeen_directory_walk(const char* path, bool recursive, 
                        void (*callback)(const char* path, const RaeenFileInfo* info, void* user_data),
                        void* user_data);

// Working directory
char* raeen_get_current_directory(void);
int raeen_set_current_directory(const char* path);

// ============================================================================
// PATH OPERATIONS
// ============================================================================

// Path manipulation
char* raeen_path_normalize(const char* path);
char* raeen_path_absolute(const char* path);
char* raeen_path_relative(const char* from, const char* to);
char* raeen_path_join(const char* base, const char* relative);
char* raeen_path_dirname(const char* path);
char* raeen_path_basename(const char* path);
char* raeen_path_extension(const char* path);
bool raeen_path_is_absolute(const char* path);
bool raeen_path_is_valid(const char* path);

// Path utilities
int raeen_path_split(const char* path, char*** components, int* count);
char* raeen_path_resolve_symlinks(const char* path);
char* raeen_path_expand_user(const char* path);  // Expand ~ to home directory
char* raeen_path_expand_vars(const char* path);  // Expand environment variables

// ============================================================================
// FILE SYSTEM OPERATIONS
// ============================================================================

// File system information
int raeen_fs_get_info(const char* path, RaeenFileSystemInfo* info);
int raeen_fs_get_free_space(const char* path, uint64_t* free_bytes);
int raeen_fs_get_total_space(const char* path, uint64_t* total_bytes);
int raeen_fs_sync(const char* path);

// Mount operations
int raeen_fs_mount(const char* device, const char* mount_point, const char* fs_type, uint32_t flags);
int raeen_fs_unmount(const char* mount_point);
int raeen_fs_list_mounts(char*** mount_points, int* count);
bool raeen_fs_is_mounted(const char* path);

// File system utilities
int raeen_fs_check(const char* device);
int raeen_fs_defrag(const char* path);
int raeen_fs_vacuum(const char* path);  // Compact free space

// ============================================================================
// SEARCH AND FILTERING
// ============================================================================

// File search criteria
typedef struct RaeenSearchCriteria {
    char* name_pattern;          // Glob pattern for name matching
    char* content_pattern;       // Text pattern for content search
    RaeenFileType type;          // File type filter
    uint64_t min_size;           // Minimum file size
    uint64_t max_size;           // Maximum file size
    uint64_t newer_than;         // Files newer than timestamp
    uint64_t older_than;         // Files older than timestamp
    uint32_t permissions_mask;   // Permission bits to match
    uint32_t attributes_mask;    // Attribute bits to match
    bool case_sensitive;         // Case sensitive matching
    bool recursive;              // Search subdirectories
} RaeenSearchCriteria;

// Search functions
int raeen_file_search(const char* directory, const RaeenSearchCriteria* criteria,
                     char*** results, int* count);
int raeen_file_find_duplicates(const char* directory, bool recursive,
                              char*** duplicate_groups, int* group_count);
int raeen_file_find_largest(const char* directory, int limit,
                           RaeenFileInfo** results, int* count);
int raeen_file_find_by_content(const char* directory, const char* pattern,
                              char*** results, int* count);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

// Batch file operations
int raeen_file_copy_multiple(const char** sources, int source_count, const char* destination);
int raeen_file_delete_multiple(const char** paths, int path_count);
int raeen_file_set_permissions_multiple(const char** paths, int path_count, uint32_t permissions);
int raeen_file_compress_multiple(const char** paths, int path_count);

// Archive operations
int raeen_archive_create(const char* archive_path, const char** files, int file_count);
int raeen_archive_extract(const char* archive_path, const char* destination);
int raeen_archive_list(const char* archive_path, char*** file_list, int* count);

// ============================================================================
// MONITORING AND EVENTS
// ============================================================================

// File system events
typedef enum {
    RAEEN_EVENT_FILE_CREATED = 0x01,
    RAEEN_EVENT_FILE_DELETED = 0x02,
    RAEEN_EVENT_FILE_MODIFIED = 0x04,
    RAEEN_EVENT_FILE_MOVED = 0x08,
    RAEEN_EVENT_FILE_ACCESSED = 0x10,
    RAEEN_EVENT_ATTR_CHANGED = 0x20,
    RAEEN_EVENT_DIR_CREATED = 0x40,
    RAEEN_EVENT_DIR_DELETED = 0x80
} RaeenFileEvent;

// Event callback
typedef void (*RaeenFileEventCallback)(const char* path, RaeenFileEvent event, void* user_data);

// File monitoring
typedef struct RaeenFileMonitor RaeenFileMonitor;

RaeenFileMonitor* raeen_file_monitor_create(const char* path, uint32_t event_mask,
                                           RaeenFileEventCallback callback, void* user_data);
int raeen_file_monitor_start(RaeenFileMonitor* monitor);
int raeen_file_monitor_stop(RaeenFileMonitor* monitor);
void raeen_file_monitor_destroy(RaeenFileMonitor* monitor);

// ============================================================================
// ADVANCED FEATURES
// ============================================================================

// Memory-mapped files
typedef struct RaeenMemoryMap RaeenMemoryMap;

RaeenMemoryMap* raeen_file_mmap(const char* path, uint32_t protection, uint32_t flags);
void* raeen_mmap_get_pointer(RaeenMemoryMap* mmap);
size_t raeen_mmap_get_size(RaeenMemoryMap* mmap);
int raeen_mmap_sync(RaeenMemoryMap* mmap);
int raeen_mmap_unmap(RaeenMemoryMap* mmap);

// File locking
typedef enum {
    RAEEN_LOCK_SHARED = 1,
    RAEEN_LOCK_EXCLUSIVE = 2
} RaeenLockType;

int raeen_file_lock(RaeenFile* file, RaeenLockType type, uint64_t start, uint64_t length);
int raeen_file_unlock(RaeenFile* file, uint64_t start, uint64_t length);
bool raeen_file_is_locked(RaeenFile* file, uint64_t start, uint64_t length);

// Extended attributes
int raeen_file_get_xattr(const char* path, const char* name, void* value, size_t size);
int raeen_file_set_xattr(const char* path, const char* name, const void* value, size_t size);
int raeen_file_remove_xattr(const char* path, const char* name);
int raeen_file_list_xattrs(const char* path, char** names, int* count);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// String utilities
char* raeen_str_duplicate(const char* str);
void raeen_str_free(char* str);
bool raeen_str_match_pattern(const char* string, const char* pattern);

// Memory utilities
void* raeen_malloc(size_t size);
void* raeen_realloc(void* ptr, size_t size);
void raeen_free(void* ptr);

// Error handling
const char* raeen_fs_error_string(int error_code);
int raeen_fs_get_last_error(void);

// Convenience macros
#define RAEEN_FILE_READ_WRITE    (RAEEN_FILE_READ | RAEEN_FILE_WRITE)
#define RAEEN_FILE_CREATE_WRITE  (RAEEN_FILE_CREATE | RAEEN_FILE_WRITE)
#define RAEEN_FILE_APPEND_WRITE  (RAEEN_FILE_APPEND | RAEEN_FILE_WRITE)

#define RAEEN_PERM_DEFAULT       (RAEEN_PERM_READ_OWNER | RAEEN_PERM_WRITE_OWNER | RAEEN_PERM_READ_GROUP | RAEEN_PERM_READ_OTHER)
#define RAEEN_PERM_EXECUTABLE    (RAEEN_PERM_DEFAULT | RAEEN_PERM_EXEC_OWNER | RAEEN_PERM_EXEC_GROUP | RAEEN_PERM_EXEC_OTHER)
#define RAEEN_PERM_PRIVATE       (RAEEN_PERM_READ_OWNER | RAEEN_PERM_WRITE_OWNER)

// Error codes
#define RAEEN_FS_SUCCESS         0
#define RAEEN_FS_ERROR_GENERIC   -1
#define RAEEN_FS_ERROR_NOT_FOUND -2
#define RAEEN_FS_ERROR_ACCESS    -3
#define RAEEN_FS_ERROR_EXISTS    -4
#define RAEEN_FS_ERROR_NO_SPACE  -5
#define RAEEN_FS_ERROR_READ_ONLY -6
#define RAEEN_FS_ERROR_INVALID   -7
#define RAEEN_FS_ERROR_TOO_BIG   -8
#define RAEEN_FS_ERROR_BUSY      -9
#define RAEEN_FS_ERROR_IO        -10

#ifdef __cplusplus
}
#endif
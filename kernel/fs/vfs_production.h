/**
 * @file vfs_production.h
 * @brief RaeenOS Production Virtual File System (VFS) Interface
 * 
 * Enterprise-grade Virtual File System implementation providing:
 * - Multi-filesystem support with unified interface
 * - Advanced caching and performance optimization
 * - Comprehensive security with ACLs and extended attributes
 * - File locking, event notification, and integrity monitoring
 * - Snapshot and versioning capabilities
 * - Cloud storage and network filesystem integration
 * 
 * Version: 2.0 - Production Ready
 * Security Level: Enterprise Grade
 * Performance Target: >95% operations under 10ms
 */

#ifndef VFS_PRODUCTION_H
#define VFS_PRODUCTION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../include/types.h"
#include "../include/errno.h"
#include "../include/time.h"
#include "../ipc/pipe.h"
#include "../sync.h"
#include "../include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// VFS Configuration Constants
#define VFS_FILENAME_MAX 256
#define VFS_PATH_MAX 4096
#define VFS_XATTR_NAME_MAX 255
#define VFS_XATTR_VALUE_MAX 65536
#define VFS_MAX_LINKS 32767
#define VFS_CACHE_ENTRIES 16384
#define VFS_DENTRY_CACHE_SIZE 8192
#define VFS_INODE_CACHE_SIZE 4096
#define VFS_BUFFER_CACHE_SIZE 32768
#define VFS_MAX_OPEN_FILES 65536
#define VFS_MAX_MOUNTS 1024
#define VFS_READAHEAD_SIZE 1048576  // 1MB default readahead

// Node Type Flags
#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_PIPE        0x03
#define VFS_CHARDEVICE  0x04
#define VFS_BLOCKDEVICE 0x05
#define VFS_SYMLINK     0x06
#define VFS_SOCKET      0x07
#define VFS_MOUNTPOINT  0x08

// Access Permission Flags
#define VFS_FLAG_READABLE   0x10
#define VFS_FLAG_WRITABLE   0x20
#define VFS_FLAG_EXECUTABLE 0x40
#define VFS_FLAG_APPEND     0x80

// Advanced Flags
#define VFS_FLAG_ENCRYPTED  0x100
#define VFS_FLAG_COMPRESSED 0x200
#define VFS_FLAG_VERSIONED  0x400
#define VFS_FLAG_IMMUTABLE  0x800
#define VFS_FLAG_SPARSE     0x1000
#define VFS_FLAG_TEMPORARY  0x2000
#define VFS_FLAG_NOATIME    0x4000
#define VFS_FLAG_SYNC       0x8000

// POSIX permissions
#define VFS_S_IFMT   0170000  // Bit mask for file type
#define VFS_S_IFSOCK 0140000  // Socket
#define VFS_S_IFLNK  0120000  // Symbolic link
#define VFS_S_IFREG  0100000  // Regular file
#define VFS_S_IFBLK  0060000  // Block device
#define VFS_S_IFDIR  0040000  // Directory
#define VFS_S_IFCHR  0020000  // Character device
#define VFS_S_IFIFO  0010000  // FIFO/pipe

#define VFS_S_ISUID  0004000  // Set UID bit
#define VFS_S_ISGID  0002000  // Set GID bit
#define VFS_S_ISVTX  0001000  // Sticky bit

#define VFS_S_IRWXU  0000700  // Owner permissions
#define VFS_S_IRUSR  0000400  // Owner read
#define VFS_S_IWUSR  0000200  // Owner write
#define VFS_S_IXUSR  0000100  // Owner execute

#define VFS_S_IRWXG  0000070  // Group permissions
#define VFS_S_IRGRP  0000040  // Group read
#define VFS_S_IWGRP  0000020  // Group write
#define VFS_S_IXGRP  0000010  // Group execute

#define VFS_S_IRWXO  0000007  // Other permissions
#define VFS_S_IROTH  0000004  // Other read
#define VFS_S_IWOTH  0000002  // Other write
#define VFS_S_IXOTH  0000001  // Other execute

// Forward declarations
struct vfs_node;
struct vfs_superblock;
struct vfs_inode;
struct vfs_dentry;
struct vfs_mount;
struct vfs_file;
struct vfs_buffer;
struct vfs_lock;
struct vfs_acl;
struct vfs_xattr;
struct vfs_watch;
struct vfs_fs_operations;

// File system types
typedef enum {
    VFS_FS_UNKNOWN = 0,
    VFS_FS_RAMFS,
    VFS_FS_FAT32,
    VFS_FS_EXT4,
    VFS_FS_NTFS,
    VFS_FS_RAEENFS,
    VFS_FS_ISO9660,
    VFS_FS_NFS,
    VFS_FS_SMB,
    VFS_FS_TMPFS,
    VFS_FS_PROCFS,
    VFS_FS_SYSFS,
    VFS_FS_DEVFS
} vfs_fs_type_t;

// Open flags
typedef enum {
    VFS_O_RDONLY    = 0x01,
    VFS_O_WRONLY    = 0x02,
    VFS_O_RDWR      = 0x03,
    VFS_O_CREAT     = 0x04,
    VFS_O_EXCL      = 0x08,
    VFS_O_TRUNC     = 0x10,
    VFS_O_APPEND    = 0x20,
    VFS_O_NONBLOCK  = 0x40,
    VFS_O_SYNC      = 0x80,
    VFS_O_DIRECT    = 0x100,
    VFS_O_DIRECTORY = 0x200,
    VFS_O_NOFOLLOW  = 0x400,
    VFS_O_LARGEFILE = 0x800,
    VFS_O_NOATIME   = 0x1000
} vfs_open_flags_t;

// Seek whence values
typedef enum {
    VFS_SEEK_SET = 0,
    VFS_SEEK_CUR = 1,
    VFS_SEEK_END = 2
} vfs_seek_whence_t;

// File lock types
typedef enum {
    VFS_LOCK_NONE = 0,
    VFS_LOCK_SHARED,
    VFS_LOCK_EXCLUSIVE,
    VFS_LOCK_MANDATORY
} vfs_lock_type_t;

// File system events
typedef enum {
    VFS_EVENT_CREATE = 0x01,
    VFS_EVENT_DELETE = 0x02,
    VFS_EVENT_MODIFY = 0x04,
    VFS_EVENT_MOVE   = 0x08,
    VFS_EVENT_ACCESS = 0x10,
    VFS_EVENT_ATTRIB = 0x20,
    VFS_EVENT_OPEN   = 0x40,
    VFS_EVENT_CLOSE  = 0x80
} vfs_event_type_t;

// POSIX ACL entry types
typedef enum {
    VFS_ACL_USER_OBJ = 0x01,
    VFS_ACL_USER     = 0x02,
    VFS_ACL_GROUP_OBJ = 0x04,
    VFS_ACL_GROUP    = 0x08,
    VFS_ACL_MASK     = 0x10,
    VFS_ACL_OTHER    = 0x20
} vfs_acl_type_t;

// Cache states
typedef enum {
    VFS_CACHE_CLEAN = 0,
    VFS_CACHE_DIRTY,
    VFS_CACHE_WRITEBACK,
    VFS_CACHE_INVALID
} vfs_cache_state_t;

// Security context structure
typedef struct vfs_security_context {
    char label[256];        // Security label
    uint32_t context_id;    // Security context ID
    uint32_t flags;         // Security flags
    uint8_t checksum[32];   // File integrity checksum
    uint64_t checksum_time; // When checksum was computed
} vfs_security_context_t;

// Time structure for file operations
typedef struct vfs_timespec {
    uint64_t tv_sec;        // Seconds since epoch
    uint64_t tv_nsec;       // Nanoseconds
} vfs_timespec_t;

// File statistics structure
typedef struct vfs_stat {
    uint64_t st_dev;        // Device ID
    uint64_t st_ino;        // Inode number
    uint32_t st_mode;       // File mode and permissions
    uint32_t st_nlink;      // Number of hard links
    uint32_t st_uid;        // User ID
    uint32_t st_gid;        // Group ID
    uint64_t st_rdev;       // Device ID (if special file)
    uint64_t st_size;       // File size in bytes
    uint32_t st_blksize;    // Block size for I/O
    uint64_t st_blocks;     // Number of blocks allocated
    vfs_timespec_t st_atime; // Access time
    vfs_timespec_t st_mtime; // Modification time
    vfs_timespec_t st_ctime; // Status change time
    vfs_timespec_t st_birthtime; // Creation time
    uint32_t st_flags;      // File flags
    uint32_t st_gen;        // File generation number
} vfs_stat_t;

// I/O vector for vectored I/O
typedef struct vfs_iovec {
    void* iov_base;         // Base address
    size_t iov_len;         // Length
} vfs_iovec_t;

// Extended attribute structure
typedef struct vfs_xattr {
    char name[VFS_XATTR_NAME_MAX + 1];
    void* value;
    size_t size;
    uint32_t flags;
    struct vfs_xattr* next;
} vfs_xattr_t;

// Access Control List entry
typedef struct vfs_acl_entry {
    vfs_acl_type_t type;
    uint32_t id;            // User/Group ID
    uint32_t permissions;   // Read/Write/Execute permissions
    struct vfs_acl_entry* next;
} vfs_acl_entry_t;

// File lock structure
typedef struct vfs_lock {
    vfs_lock_type_t type;
    uint64_t start;
    uint64_t end;
    uint32_t pid;           // Process ID holding the lock
    struct vfs_lock* next;
    spinlock_t lock;
} vfs_lock_t;

// Buffer cache entry
typedef struct vfs_buffer {
    uint64_t block_num;     // Block number
    uint64_t device_id;     // Device identifier
    uint32_t size;          // Buffer size
    uint32_t flags;         // Buffer flags
    uint32_t ref_count;     // Reference count
    vfs_cache_state_t state; // Cache state
    void* data;             // Buffer data
    struct vfs_buffer* next;
    struct vfs_buffer* prev;
    struct vfs_buffer* hash_next; // Hash table linkage
    spinlock_t lock;
    uint64_t last_access;   // For LRU eviction
} vfs_buffer_t;

// Directory entry cache
typedef struct vfs_dentry {
    char name[VFS_FILENAME_MAX];
    struct vfs_inode* inode;
    struct vfs_dentry* parent;
    struct vfs_dentry* next_sibling;
    struct vfs_dentry* first_child;
    uint32_t ref_count;
    uint32_t flags;
    uint32_t hash;          // Name hash for quick lookup
    spinlock_t lock;
    uint64_t last_access;   // For LRU eviction
} vfs_dentry_t;

// Inode structure
typedef struct vfs_inode {
    uint64_t ino;           // Inode number
    uint32_t mode;          // File mode and permissions
    uint32_t uid;           // User ID
    uint32_t gid;           // Group ID
    uint64_t size;          // File size
    uint32_t nlink;         // Number of hard links
    uint32_t flags;         // File flags
    vfs_timespec_t atime;   // Access time
    vfs_timespec_t mtime;   // Modification time
    vfs_timespec_t ctime;   // Status change time
    vfs_timespec_t birthtime; // Creation time
    
    // Security and extended attributes
    vfs_security_context_t security;
    vfs_acl_entry_t* acl;
    vfs_xattr_t* xattrs;
    
    // File locks
    vfs_lock_t* locks;
    
    // Filesystem operations
    struct vfs_inode_operations* ops;
    
    // Superblock reference
    struct vfs_superblock* sb;
    
    // Filesystem-specific data
    void* private_data;
    
    // Reference counting and caching
    uint32_t ref_count;
    bool dirty;
    spinlock_t lock;
    
    // Hash table linkage
    struct vfs_inode* hash_next;
    uint64_t last_access;   // For LRU eviction
    
    // Read-ahead state
    uint64_t ra_offset;     // Last read-ahead offset
    uint32_t ra_size;       // Read-ahead window size
} vfs_inode_t;

// File structure
typedef struct vfs_file {
    struct vfs_inode* inode;
    struct vfs_dentry* dentry;
    uint64_t position;
    uint32_t flags;
    uint32_t mode;
    
    // File operations
    struct vfs_file_operations* ops;
    
    // Private data for filesystem
    void* private_data;
    
    // Reference counting
    uint32_t ref_count;
    spinlock_t lock;
    
    // Read-ahead context
    uint64_t ra_offset;
    uint32_t ra_size;
    
    // Event notification
    struct vfs_watch* watchers;
} vfs_file_t;

// Mount point structure
typedef struct vfs_mount {
    struct vfs_superblock* sb;
    struct vfs_dentry* mountpoint;
    struct vfs_dentry* root;
    char device[256];
    char fstype[64];
    char options[512];
    uint32_t flags;
    struct vfs_mount* next;
    spinlock_t lock;
    uint32_t ref_count;
} vfs_mount_t;

// Superblock structure
typedef struct vfs_superblock {
    vfs_fs_type_t type;
    uint64_t magic;
    uint32_t blocksize;
    uint64_t blocks;
    uint64_t free_blocks;
    uint64_t inodes;
    uint64_t free_inodes;
    uint32_t flags;
    
    // Root inode
    struct vfs_inode* root_inode;
    
    // Filesystem operations
    struct vfs_super_operations* ops;
    
    // Device information
    void* device_data;
    
    // Private filesystem data
    void* private_data;
    
    // Dirty inodes list
    struct vfs_inode* dirty_inodes;
    
    spinlock_t lock;
    uint32_t ref_count;
    
    // Statistics
    uint64_t read_operations;
    uint64_t write_operations;
    uint64_t lookup_operations;
} vfs_superblock_t;

// File system watcher
typedef struct vfs_watch {
    uint32_t mask;          // Event mask
    void (*callback)(struct vfs_watch* watch, vfs_event_type_t event, const char* path);
    void* user_data;
    struct vfs_watch* next;
    spinlock_t lock;
} vfs_watch_t;

// VFS Operation Function Pointers

// File operations
typedef struct vfs_file_operations {
    ssize_t (*read)(struct vfs_file* file, void* buffer, size_t count, off_t* offset);
    ssize_t (*write)(struct vfs_file* file, const void* buffer, size_t count, off_t* offset);
    int (*open)(struct vfs_inode* inode, struct vfs_file* file);
    int (*close)(struct vfs_file* file);
    off_t (*seek)(struct vfs_file* file, off_t offset, int whence);
    int (*ioctl)(struct vfs_file* file, unsigned int cmd, unsigned long arg);
    int (*mmap)(struct vfs_file* file, void* addr, size_t length, int prot, int flags, off_t offset);
    int (*flush)(struct vfs_file* file);
    int (*fsync)(struct vfs_file* file, int datasync);
    int (*lock)(struct vfs_file* file, int cmd, struct vfs_lock* lock);
    ssize_t (*readv)(struct vfs_file* file, const struct vfs_iovec* iov, int count);
    ssize_t (*writev)(struct vfs_file* file, const struct vfs_iovec* iov, int count);
    int (*poll)(struct vfs_file* file, void* poll_table);
    ssize_t (*sendfile)(struct vfs_file* out_file, struct vfs_file* in_file, off_t* offset, size_t count);
} vfs_file_operations_t;

// Directory operations  
typedef void* (*filldir_t)(void* dirent, const char* name, int namelen, off_t offset, uint64_t ino, unsigned int d_type);

// Inode operations
typedef struct vfs_inode_operations {
    struct vfs_dentry* (*lookup)(struct vfs_inode* dir, struct vfs_dentry* dentry);
    int (*create)(struct vfs_inode* dir, struct vfs_dentry* dentry, int mode);
    int (*link)(struct vfs_dentry* old_dentry, struct vfs_inode* dir, struct vfs_dentry* dentry);
    int (*unlink)(struct vfs_inode* dir, struct vfs_dentry* dentry);
    int (*symlink)(struct vfs_inode* dir, struct vfs_dentry* dentry, const char* target);
    int (*mkdir)(struct vfs_inode* dir, struct vfs_dentry* dentry, int mode);
    int (*rmdir)(struct vfs_inode* dir, struct vfs_dentry* dentry);
    int (*mknod)(struct vfs_inode* dir, struct vfs_dentry* dentry, int mode, dev_t dev);
    int (*rename)(struct vfs_inode* old_dir, struct vfs_dentry* old_dentry,
                  struct vfs_inode* new_dir, struct vfs_dentry* new_dentry);
    int (*readlink)(struct vfs_dentry* dentry, char* buffer, int buflen);
    int (*permission)(struct vfs_inode* inode, int mask);
    int (*setattr)(struct vfs_dentry* dentry, struct vfs_stat* attr);
    int (*getattr)(struct vfs_dentry* dentry, struct vfs_stat* stat);
    ssize_t (*listxattr)(struct vfs_dentry* dentry, char* list, size_t size);
    int (*getxattr)(struct vfs_dentry* dentry, const char* name, void* value, size_t size);
    int (*setxattr)(struct vfs_dentry* dentry, const char* name, const void* value, size_t size, int flags);
    int (*removexattr)(struct vfs_dentry* dentry, const char* name);
    int (*readdir)(struct vfs_file* file, void* dirent, filldir_t filldir);
} vfs_inode_operations_t;

// Superblock operations
typedef struct vfs_super_operations {
    struct vfs_inode* (*alloc_inode)(struct vfs_superblock* sb);
    void (*destroy_inode)(struct vfs_inode* inode);
    void (*dirty_inode)(struct vfs_inode* inode);
    int (*write_inode)(struct vfs_inode* inode, int sync);
    void (*drop_inode)(struct vfs_inode* inode);
    void (*delete_inode)(struct vfs_inode* inode);
    void (*put_super)(struct vfs_superblock* sb);
    int (*sync_fs)(struct vfs_superblock* sb, int wait);
    int (*statfs)(struct vfs_superblock* sb, struct statfs* buf);
    int (*remount_fs)(struct vfs_superblock* sb, int* flags, char* data);
    void (*clear_inode)(struct vfs_inode* inode);
    int (*show_options)(void* seq_file, struct vfs_mount* mnt);
    int (*freeze_fs)(struct vfs_superblock* sb);
    int (*unfreeze_fs)(struct vfs_superblock* sb);
} vfs_super_operations_t;

// Filesystem type operations
typedef struct vfs_fs_operations {
    const char* name;
    struct vfs_superblock* (*mount)(const char* device, uint32_t flags, const void* data);
    void (*unmount)(struct vfs_superblock* sb);
    int (*get_sb)(const char* device, uint32_t flags, const void* data, struct vfs_superblock** sb);
    void (*kill_sb)(struct vfs_superblock* sb);
    struct vfs_fs_operations* next;
} vfs_fs_operations_t;

// Legacy compatibility structures
typedef uint32_t (*vfs_read_t)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef uint32_t (*vfs_write_t)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef void (*vfs_open_t)(struct vfs_node* node, uint32_t flags);
typedef void (*vfs_close_t)(struct vfs_node* node);
typedef struct dirent* (*vfs_readdir_t)(struct vfs_node* node, uint32_t index);
typedef struct vfs_node* (*vfs_finddir_t)(struct vfs_node* node, const char* name);
typedef struct vfs_node* (*vfs_create_t)(struct vfs_node* node, const char* name, uint32_t flags);

// Directory entry for legacy compatibility
struct dirent {
    char name[VFS_FILENAME_MAX];
    uint32_t inode_num;
    uint8_t type;
};

// Legacy VFS node structure for backward compatibility
typedef struct vfs_node {
    char name[VFS_FILENAME_MAX]; // The name of this node
    uint32_t flags;              // Flags (file, directory, etc.)
    uint32_t inode;              // Inode number, unique within a filesystem
    uint32_t length;             // Length of the file in bytes
    uint32_t permissions;        // Access permissions

    // VFS operations (legacy)
    vfs_read_t read;
    vfs_write_t write;
    vfs_open_t open;
    vfs_close_t close;
    vfs_readdir_t readdir;
    vfs_finddir_t finddir;
    vfs_create_t create;

    // For mountpoints
    struct vfs_node* mounted_at;

    // For pipes
    pipe_t* pipe;

    // Modern VFS integration
    struct vfs_inode* modern_inode;
    struct vfs_dentry* dentry;

    // Filesystem-specific private data
    void* fs_private_data;
} vfs_node_t;

// Global VFS state
extern vfs_node_t* vfs_root;              // Legacy root node
extern vfs_dentry_t* vfs_root_dentry;     // Modern root dentry
extern vfs_mount_t* vfs_mount_list;       // Mount point list
extern vfs_buffer_t* vfs_buffer_cache;    // Buffer cache
extern vfs_inode_t* vfs_inode_cache;      // Inode cache
extern vfs_dentry_t* vfs_dentry_cache;    // Dentry cache
extern spinlock_t vfs_lock;               // Global VFS lock
extern vfs_fs_operations_t* vfs_filesystems; // Registered filesystems

// Statistics and monitoring
typedef struct vfs_stats {
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t buffer_reads;
    uint64_t buffer_writes;
    uint64_t inode_allocations;
    uint64_t dentry_allocations;
    uint64_t file_operations;
    uint64_t directory_operations;
    uint64_t lookup_operations;
    uint64_t sync_operations;
    uint64_t readahead_hits;
    uint64_t readahead_misses;
} vfs_stats_t;

extern vfs_stats_t vfs_stats;

// Core VFS API Functions

// Initialization and shutdown
int vfs_init(void);
void vfs_shutdown(void);

// Filesystem registration
int vfs_register_filesystem(vfs_fs_operations_t* fs_ops);
int vfs_unregister_filesystem(const char* name);
vfs_fs_operations_t* vfs_find_filesystem(const char* name);

// Mount and unmount operations
int vfs_mount(const char* device, const char* mountpoint, const char* fstype, uint32_t flags, const void* data);
int vfs_unmount(const char* mountpoint, uint32_t flags);
vfs_mount_t* vfs_find_mount(const char* path);
int vfs_get_mounts(vfs_mount_t** mounts, size_t* count);

// Path resolution
vfs_dentry_t* vfs_lookup(const char* path);
vfs_dentry_t* vfs_parent_lookup(vfs_dentry_t* dentry, const char* name);
int vfs_resolve_path(const char* path, vfs_dentry_t** result);
char* vfs_get_absolute_path(vfs_dentry_t* dentry);

// File operations
vfs_file_t* vfs_open(const char* path, int flags, int mode);
int vfs_close(vfs_file_t* file);
ssize_t vfs_read_file(vfs_file_t* file, void* buffer, size_t count);
ssize_t vfs_write_file(vfs_file_t* file, const void* buffer, size_t count);
off_t vfs_seek_file(vfs_file_t* file, off_t offset, int whence);
int vfs_truncate(const char* path, off_t length);
int vfs_sync_file(vfs_file_t* file);
ssize_t vfs_readv(vfs_file_t* file, const vfs_iovec_t* iov, int count);
ssize_t vfs_writev(vfs_file_t* file, const vfs_iovec_t* iov, int count);

// Directory operations
int vfs_mkdir(const char* path, int mode);
int vfs_rmdir(const char* path);
vfs_file_t* vfs_opendir(const char* path);
struct dirent* vfs_readdir_entry(vfs_file_t* dir);
int vfs_closedir(vfs_file_t* dir);
int vfs_scandir(const char* path, struct dirent*** namelist, 
               int (*filter)(const struct dirent*),
               int (*compar)(const struct dirent**, const struct dirent**));

// File management
int vfs_create_file(const char* path, int mode);
int vfs_unlink(const char* path);
int vfs_link(const char* oldpath, const char* newpath);
int vfs_symlink(const char* target, const char* linkpath);
int vfs_rename(const char* oldpath, const char* newpath);
ssize_t vfs_readlink(const char* path, char* buffer, size_t size);
int vfs_mknod(const char* path, int mode, dev_t dev);

// File attributes and permissions
int vfs_stat(const char* path, vfs_stat_t* stat);
int vfs_lstat(const char* path, vfs_stat_t* stat);  // Don't follow symlinks
int vfs_fstat(vfs_file_t* file, vfs_stat_t* stat);
int vfs_chmod(const char* path, int mode);
int vfs_chown(const char* path, uid_t uid, gid_t gid);
int vfs_lchown(const char* path, uid_t uid, gid_t gid);  // Don't follow symlinks
int vfs_utime(const char* path, const vfs_timespec_t* times);
int vfs_access(const char* path, int mode);

// Extended attributes
ssize_t vfs_listxattr(const char* path, char* list, size_t size);
ssize_t vfs_getxattr(const char* path, const char* name, void* value, size_t size);
int vfs_setxattr(const char* path, const char* name, const void* value, size_t size, int flags);
int vfs_removexattr(const char* path, const char* name);
ssize_t vfs_llistxattr(const char* path, char* list, size_t size);  // Don't follow symlinks
ssize_t vfs_lgetxattr(const char* path, const char* name, void* value, size_t size);
int vfs_lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags);
int vfs_lremovexattr(const char* path, const char* name);

// Access Control Lists
int vfs_getacl(const char* path, vfs_acl_entry_t** acl);
int vfs_setacl(const char* path, const vfs_acl_entry_t* acl);
int vfs_check_permission(vfs_inode_t* inode, int mask, uid_t uid, gid_t gid);
void vfs_free_acl(vfs_acl_entry_t* acl);

// File locking
int vfs_lock_file(vfs_file_t* file, vfs_lock_type_t type, off_t start, off_t end);
int vfs_unlock_file(vfs_file_t* file, off_t start, off_t end);
int vfs_test_lock(vfs_file_t* file, vfs_lock_t* lock);
void vfs_clear_locks(vfs_file_t* file);

// File system events and watching
vfs_watch_t* vfs_add_watch(const char* path, uint32_t mask, 
                          void (*callback)(vfs_watch_t*, vfs_event_type_t, const char*),
                          void* user_data);
int vfs_remove_watch(vfs_watch_t* watch);
void vfs_notify_event(const char* path, vfs_event_type_t event);

// Cache management
vfs_buffer_t* vfs_get_buffer(uint64_t device_id, uint64_t block_num, size_t size);
void vfs_put_buffer(vfs_buffer_t* buffer);
int vfs_sync_buffers(uint64_t device_id);
void vfs_invalidate_buffers(uint64_t device_id);
vfs_inode_t* vfs_get_inode(vfs_superblock_t* sb, uint64_t ino);
void vfs_put_inode(vfs_inode_t* inode);
vfs_dentry_t* vfs_get_dentry(const char* name, vfs_dentry_t* parent);
void vfs_put_dentry(vfs_dentry_t* dentry);
int vfs_sync_all(void);

// Memory management for VFS structures
vfs_inode_t* vfs_alloc_inode(vfs_superblock_t* sb);
void vfs_free_inode(vfs_inode_t* inode);
vfs_dentry_t* vfs_alloc_dentry(const char* name);
void vfs_free_dentry(vfs_dentry_t* dentry);
vfs_file_t* vfs_alloc_file(void);
void vfs_free_file(vfs_file_t* file);
vfs_buffer_t* vfs_alloc_buffer(size_t size);
void vfs_free_buffer(vfs_buffer_t* buffer);

// Performance and statistics
int vfs_get_stats(vfs_stats_t* stats);
void vfs_reset_stats(void);
int vfs_cache_pressure(void);
void vfs_cache_shrink(size_t target);
void vfs_readahead(vfs_file_t* file, off_t offset, size_t count);

// Integrity and security
int vfs_verify_integrity(const char* path);
int vfs_compute_checksum(const char* path, uint8_t* checksum, size_t checksum_size);
int vfs_encrypt_file(const char* path, const char* key);
int vfs_decrypt_file(const char* path, const char* key);
int vfs_secure_delete(const char* path);

// Snapshot and versioning
int vfs_create_snapshot(const char* path, const char* snapshot_name);
int vfs_restore_snapshot(const char* path, const char* snapshot_name);
int vfs_list_snapshots(const char* path, char** snapshots, size_t* count);
int vfs_delete_snapshot(const char* path, const char* snapshot_name);
int vfs_get_file_versions(const char* path, vfs_stat_t** versions, size_t* count);

// Network filesystem support
int vfs_register_network_fs(const char* fstype, vfs_fs_operations_t* ops);
int vfs_mount_network_fs(const char* remote_path, const char* local_path, 
                        const char* fstype, const char* credentials);

// Compression and deduplication
int vfs_compress_file(const char* path, const char* algorithm);
int vfs_decompress_file(const char* path);
int vfs_deduplicate_files(const char** paths, size_t count);
int vfs_find_duplicates(const char* directory, char*** duplicates, size_t* count);

// Legacy compatibility functions
vfs_node_t* vfs_find(const char* path);
uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
vfs_node_t* vfs_create(vfs_node_t* parent, const char* name, uint32_t flags);
void vfs_open_legacy(vfs_node_t* node, uint32_t flags);
void vfs_close_legacy(vfs_node_t* node);
struct dirent* vfs_readdir(vfs_node_t* node, uint32_t index);
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name);

// Utility functions
uint32_t vfs_hash_string(const char* str);
int vfs_path_normalize(const char* path, char* normalized, size_t size);
bool vfs_path_is_absolute(const char* path);
char* vfs_path_join(const char* dir, const char* name);
char* vfs_path_dirname(const char* path);
char* vfs_path_basename(const char* path);

// Error codes specific to VFS
#define VFS_SUCCESS            0
#define VFS_ERR_NOT_FOUND     -2001
#define VFS_ERR_PERMISSION    -2002
#define VFS_ERR_EXISTS        -2003
#define VFS_ERR_NOT_DIR       -2004
#define VFS_ERR_IS_DIR        -2005
#define VFS_ERR_NO_SPACE      -2006
#define VFS_ERR_READ_ONLY     -2007
#define VFS_ERR_BUSY          -2008
#define VFS_ERR_INVALID_PATH  -2009
#define VFS_ERR_TOO_MANY_LINKS -2010
#define VFS_ERR_NAME_TOO_LONG -2011
#define VFS_ERR_NOT_SUPPORTED -2012
#define VFS_ERR_CORRUPTED     -2013
#define VFS_ERR_LOCKED        -2014
#define VFS_ERR_TIMEOUT       -2015
#define VFS_ERR_NO_MEMORY     -2016
#define VFS_ERR_INVALID_ARG   -2017
#define VFS_ERR_NOT_EMPTY     -2018
#define VFS_ERR_CROSS_DEVICE  -2019
#define VFS_ERR_IO_ERROR      -2020

#ifdef __cplusplus
}
#endif

#endif // VFS_PRODUCTION_H
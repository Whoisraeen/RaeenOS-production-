#ifndef FILESYSTEM_INTERFACE_H
#define FILESYSTEM_INTERFACE_H

/**
 * @file filesystem_interface.h
 * @brief Comprehensive Filesystem Interface for RaeenOS
 * 
 * This interface defines the Virtual File System (VFS) APIs and
 * RaeenFS-specific operations, providing a unified filesystem
 * abstraction for all storage operations in RaeenOS.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "driver_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

// Filesystem API version
#define FILESYSTEM_API_VERSION 1

// File system limits
#define VFS_NAME_MAX           256
#define VFS_PATH_MAX           4096
#define VFS_SYMLINK_MAX        1024
#define VFS_XATTR_NAME_MAX     256
#define VFS_XATTR_VALUE_MAX    65536
#define VFS_MAX_FILESYSTEMS    64
#define VFS_MAX_MOUNTS         1024

// File types
#define VFS_TYPE_UNKNOWN       0x0000
#define VFS_TYPE_REGULAR       0x8000  // Regular file
#define VFS_TYPE_DIRECTORY     0x4000  // Directory
#define VFS_TYPE_CHARDEV       0x2000  // Character device
#define VFS_TYPE_BLOCKDEV      0x6000  // Block device
#define VFS_TYPE_FIFO          0x1000  // Named pipe (FIFO)
#define VFS_TYPE_SYMLINK       0xA000  // Symbolic link
#define VFS_TYPE_SOCKET        0xC000  // Socket

// File permissions (POSIX compatible)
#define VFS_PERM_OWNER_READ    0x0100  // Owner read
#define VFS_PERM_OWNER_WRITE   0x0080  // Owner write
#define VFS_PERM_OWNER_EXEC    0x0040  // Owner execute
#define VFS_PERM_GROUP_READ    0x0020  // Group read
#define VFS_PERM_GROUP_WRITE   0x0010  // Group write
#define VFS_PERM_GROUP_EXEC    0x0008  // Group execute
#define VFS_PERM_OTHER_READ    0x0004  // Other read
#define VFS_PERM_OTHER_WRITE   0x0002  // Other write
#define VFS_PERM_OTHER_EXEC    0x0001  // Other execute
#define VFS_PERM_SETUID        0x0800  // Set UID
#define VFS_PERM_SETGID        0x0400  // Set GID
#define VFS_PERM_STICKY        0x0200  // Sticky bit

// File flags
#define VFS_FLAG_READ          (1 << 0)  // Readable
#define VFS_FLAG_WRITE         (1 << 1)  // Writable
#define VFS_FLAG_APPEND        (1 << 2)  // Append mode
#define VFS_FLAG_CREATE        (1 << 3)  // Create if not exists
#define VFS_FLAG_EXCLUSIVE     (1 << 4)  // Exclusive creation
#define VFS_FLAG_TRUNCATE      (1 << 5)  // Truncate on open
#define VFS_FLAG_NONBLOCK      (1 << 6)  // Non-blocking I/O
#define VFS_FLAG_SYNC          (1 << 7)  // Synchronous I/O
#define VFS_FLAG_DIRECT        (1 << 8)  // Direct I/O (bypass cache)
#define VFS_FLAG_LARGEFILE     (1 << 9)  // Large file support
#define VFS_FLAG_NOATIME       (1 << 10) // Don't update access time
#define VFS_FLAG_CLOEXEC       (1 << 11) // Close on exec
#define VFS_FLAG_TEMPORARY     (1 << 12) // Temporary file

// Mount flags
#define VFS_MOUNT_READONLY     (1 << 0)  // Read-only mount
#define VFS_MOUNT_NOSUID       (1 << 1)  // No SUID/SGID
#define VFS_MOUNT_NODEV        (1 << 2)  // No device files
#define VFS_MOUNT_NOEXEC       (1 << 3)  // No execution
#define VFS_MOUNT_SYNC         (1 << 4)  // Synchronous I/O
#define VFS_MOUNT_REMOUNT      (1 << 5)  // Remount
#define VFS_MOUNT_BIND         (1 << 6)  // Bind mount
#define VFS_MOUNT_MOVE         (1 << 7)  // Move mount
#define VFS_MOUNT_NOATIME      (1 << 8)  // No access time updates
#define VFS_MOUNT_RELATIME     (1 << 9)  // Relative access time

// Seek whence values
#define VFS_SEEK_SET           0         // Absolute position
#define VFS_SEEK_CUR           1         // Relative to current
#define VFS_SEEK_END           2         // Relative to end

// Forward declarations
typedef struct vfs_node vfs_node_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_dentry vfs_dentry_t;
typedef struct vfs_inode vfs_inode_t;
typedef struct vfs_superblock vfs_superblock_t;
typedef struct vfs_filesystem vfs_filesystem_t;
typedef struct vfs_mount vfs_mount_t;

// File attributes structure
typedef struct vfs_attributes {
    uint32_t mode;              // File type and permissions
    uint32_t uid;               // Owner user ID
    uint32_t gid;               // Owner group ID
    uint64_t size;              // File size in bytes
    uint64_t blocks;            // Number of 512-byte blocks
    uint32_t block_size;        // Preferred block size
    
    // Timestamps (nanoseconds since epoch)
    uint64_t atime;             // Access time
    uint64_t mtime;             // Modification time
    uint64_t ctime;             // Status change time
    uint64_t btime;             // Birth/creation time
    
    uint32_t nlink;             // Number of hard links
    uint32_t rdev;              // Device ID (if special file)
    uint64_t inode;             // Inode number
    uint32_t dev;               // Device containing file
    
    // Extended attributes
    uint32_t flags;             // File flags
    uint32_t generation;        // File generation number
    
    // RaeenFS specific
    uint32_t compression;       // Compression algorithm
    uint32_t encryption;        // Encryption status
    uint32_t checksum;          // File checksum
} vfs_attributes_t;

// Directory entry structure
typedef struct vfs_dirent {
    uint64_t inode;             // Inode number
    off_t offset;               // Offset to next entry
    uint16_t record_length;     // Length of this record
    uint8_t type;               // File type
    char name[];                // Filename (null-terminated)
} vfs_dirent_t;

// Extended attribute structure
typedef struct vfs_xattr {
    char name[VFS_XATTR_NAME_MAX];      // Attribute name
    void* value;                        // Attribute value
    size_t value_size;                  // Value size
    uint32_t flags;                     // Attribute flags
} vfs_xattr_t;

// File lock structure
typedef struct vfs_flock {
    uint16_t type;              // Lock type (F_RDLCK, F_WRLCK, F_UNLCK)
    uint16_t whence;            // SEEK_SET, SEEK_CUR, or SEEK_END
    off_t start;                // Starting offset
    off_t length;               // Length (0 = to EOF)
    uint32_t pid;               // Process ID holding lock
} vfs_flock_t;

// Filesystem statistics
typedef struct vfs_statfs {
    uint32_t type;              // Filesystem type
    uint32_t block_size;        // Block size
    uint64_t total_blocks;      // Total blocks
    uint64_t free_blocks;       // Free blocks
    uint64_t available_blocks;  // Available blocks (non-root)
    uint64_t total_inodes;      // Total inodes
    uint64_t free_inodes;       // Free inodes
    uint64_t available_inodes;  // Available inodes
    uint32_t max_name_length;   // Maximum filename length
    uint32_t flags;             // Mount flags
    char fs_type[32];           // Filesystem type name
} vfs_statfs_t;

// VFS inode structure
struct vfs_inode {
    uint64_t inode_number;      // Inode number
    uint32_t ref_count;         // Reference count
    vfs_attributes_t attr;      // File attributes
    
    // Operations
    struct vfs_inode_operations* ops;
    
    // Filesystem specific data
    void* private_data;
    
    // Superblock
    vfs_superblock_t* sb;
    
    // Dirty flag and synchronization
    bool dirty;
    void* lock;
    
    // Address space for page cache
    void* address_space;
};

// VFS directory entry (dentry)
struct vfs_dentry {
    char name[VFS_NAME_MAX];    // Entry name
    vfs_inode_t* inode;         // Associated inode
    vfs_dentry_t* parent;       // Parent directory
    vfs_dentry_t* children;     // Child entries
    vfs_dentry_t* sibling;      // Next sibling
    
    uint32_t ref_count;         // Reference count
    uint32_t flags;             // Dentry flags
    
    // Operations
    struct vfs_dentry_operations* ops;
    
    void* private_data;
};

// VFS file structure  
struct vfs_file {
    vfs_dentry_t* dentry;       // Directory entry
    vfs_inode_t* inode;         // File inode
    off_t position;             // Current file position
    uint32_t flags;             // File flags
    uint32_t mode;              // Open mode
    
    // Operations
    struct vfs_file_operations* ops;
    
    // Reference counting
    uint32_t ref_count;
    
    // Private data
    void* private_data;
};

// VFS superblock structure
struct vfs_superblock {
    uint32_t magic;             // Filesystem magic number
    vfs_filesystem_t* fs_type;  // Filesystem type
    device_t* device;           // Block device
    
    // Filesystem geometry
    uint32_t block_size;        // Block size
    uint32_t inode_size;        // Inode size
    uint64_t total_blocks;      // Total blocks
    uint64_t free_blocks;       // Free blocks
    uint64_t total_inodes;      // Total inodes
    uint64_t free_inodes;       // Free inodes
    
    // Root inode
    vfs_inode_t* root_inode;
    
    // Filesystem flags
    uint32_t flags;
    uint32_t mount_flags;
    
    // Operations
    struct vfs_superblock_operations* ops;
    
    // Dirty flag and synchronization
    bool dirty;
    void* lock;
    
    // Private data
    void* private_data;
};

// VFS mount structure
struct vfs_mount {
    vfs_superblock_t* sb;       // Superblock
    vfs_dentry_t* mount_point;  // Mount point
    vfs_dentry_t* root;         // Root of mounted filesystem
    
    char* device_name;          // Device name
    char* fs_type;              // Filesystem type
    uint32_t flags;             // Mount flags
    
    // Reference counting
    uint32_t ref_count;
    
    // Mount hierarchy
    vfs_mount_t* parent;        // Parent mount
    vfs_mount_t* children;      // Child mounts
    vfs_mount_t* sibling;       // Sibling mounts
    
    void* private_data;
};

// VFS filesystem type structure
struct vfs_filesystem {
    char name[64];              // Filesystem name
    uint32_t flags;             // Filesystem flags
    
    // Operations
    struct vfs_filesystem_operations* ops;
    
    // Module information
    void* module;               // Filesystem module
    
    void* private_data;
};

// VFS operations structures

// File operations
typedef struct vfs_file_operations {
    ssize_t (*read)(vfs_file_t* file, void* buffer, size_t count, off_t* offset);
    ssize_t (*write)(vfs_file_t* file, const void* buffer, size_t count, off_t* offset);
    off_t (*seek)(vfs_file_t* file, off_t offset, int whence);
    int (*ioctl)(vfs_file_t* file, unsigned long cmd, void* arg);
    int (*flush)(vfs_file_t* file);
    int (*fsync)(vfs_file_t* file, int datasync);
    int (*lock)(vfs_file_t* file, vfs_flock_t* lock);
    int (*flock)(vfs_file_t* file, int operation);
    int (*mmap)(vfs_file_t* file, void** addr, size_t length, int prot, int flags, off_t offset);
    int (*open)(vfs_inode_t* inode, vfs_file_t* file);
    int (*release)(vfs_inode_t* inode, vfs_file_t* file);
    
    // Asynchronous I/O
    int (*aio_read)(vfs_file_t* file, void* buffer, size_t count, off_t offset, void* callback);
    int (*aio_write)(vfs_file_t* file, const void* buffer, size_t count, off_t offset, void* callback);
    
    // Polling
    int (*poll)(vfs_file_t* file, void* poll_table);
} vfs_file_ops_t;

// Inode operations
typedef struct vfs_inode_operations {
    int (*create)(vfs_inode_t* dir, vfs_dentry_t* dentry, uint32_t mode);
    vfs_dentry_t* (*lookup)(vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*link)(vfs_dentry_t* old_dentry, vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*unlink)(vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*symlink)(vfs_inode_t* dir, vfs_dentry_t* dentry, const char* symname);
    int (*mkdir)(vfs_inode_t* dir, vfs_dentry_t* dentry, uint32_t mode);
    int (*rmdir)(vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*mknod)(vfs_inode_t* dir, vfs_dentry_t* dentry, uint32_t mode, uint32_t rdev);
    int (*rename)(vfs_inode_t* old_dir, vfs_dentry_t* old_dentry,
                  vfs_inode_t* new_dir, vfs_dentry_t* new_dentry);
    
    // Attribute operations
    int (*getattr)(vfs_dentry_t* dentry, vfs_attributes_t* attr);
    int (*setattr)(vfs_dentry_t* dentry, vfs_attributes_t* attr);
    
    // Extended attributes
    ssize_t (*getxattr)(vfs_dentry_t* dentry, const char* name, void* value, size_t size);
    int (*setxattr)(vfs_dentry_t* dentry, const char* name, const void* value, size_t size, int flags);
    ssize_t (*listxattr)(vfs_dentry_t* dentry, char* list, size_t size);
    int (*removexattr)(vfs_dentry_t* dentry, const char* name);
    
    // Directory operations
    int (*readdir)(vfs_file_t* file, void* buffer, size_t count, off_t* offset);
    
    // Symbolic link operations
    int (*readlink)(vfs_dentry_t* dentry, char* buffer, size_t buflen);
    int (*follow_link)(vfs_dentry_t* dentry, void** cookie);
    void (*put_link)(vfs_dentry_t* dentry, void* cookie);
    
    // Permission checking
    int (*permission)(vfs_inode_t* inode, int mask);
    
    // Truncation
    int (*truncate)(vfs_inode_t* inode, off_t size);
} vfs_inode_ops_t;

// Dentry operations
typedef struct vfs_dentry_operations {
    int (*revalidate)(vfs_dentry_t* dentry);
    int (*hash)(vfs_dentry_t* dentry, const char* name);
    int (*compare)(vfs_dentry_t* dentry, const char* name1, const char* name2);
    int (*delete)(vfs_dentry_t* dentry);
    void (*release)(vfs_dentry_t* dentry);
    void (*put)(vfs_dentry_t* dentry);
} vfs_dentry_ops_t;

// Superblock operations
typedef struct vfs_superblock_operations {
    vfs_inode_t* (*alloc_inode)(vfs_superblock_t* sb);
    void (*destroy_inode)(vfs_inode_t* inode);
    void (*dirty_inode)(vfs_inode_t* inode);
    int (*write_inode)(vfs_inode_t* inode, int wait);
    void (*drop_inode)(vfs_inode_t* inode);
    void (*delete_inode)(vfs_inode_t* inode);
    void (*put_super)(vfs_superblock_t* sb);
    int (*write_super)(vfs_superblock_t* sb);
    int (*sync_fs)(vfs_superblock_t* sb, int wait);
    int (*freeze_fs)(vfs_superblock_t* sb);
    int (*unfreeze_fs)(vfs_superblock_t* sb);
    int (*statfs)(vfs_dentry_t* dentry, vfs_statfs_t* statfs);
    int (*remount_fs)(vfs_superblock_t* sb, int* flags, char* data);
    void (*clear_inode)(vfs_inode_t* inode);
    int (*show_options)(vfs_mount_t* mount, void* seq_file);
} vfs_superblock_ops_t;

// Filesystem operations
typedef struct vfs_filesystem_operations {
    vfs_dentry_t* (*mount)(vfs_filesystem_t* fs_type, int flags, 
                           const char* dev_name, void* data);
    void (*kill_sb)(vfs_superblock_t* sb);
    void (*unmount)(vfs_superblock_t* sb);
} vfs_filesystem_ops_t;

// VFS main operations structure
typedef struct vfs_operations {
    // Initialization
    int (*init)(void);
    void (*cleanup)(void);
    
    // File operations
    int (*open)(const char* path, int flags, uint32_t mode, vfs_file_t** file);
    int (*close)(vfs_file_t* file);
    ssize_t (*read)(vfs_file_t* file, void* buffer, size_t count);
    ssize_t (*write)(vfs_file_t* file, const void* buffer, size_t count);
    off_t (*seek)(vfs_file_t* file, off_t offset, int whence);
    int (*ioctl)(vfs_file_t* file, unsigned long cmd, void* arg);
    int (*stat)(const char* path, vfs_attributes_t* attr);
    int (*fstat)(vfs_file_t* file, vfs_attributes_t* attr);
    int (*access)(const char* path, int mode);
    
    // Directory operations
    int (*mkdir)(const char* path, uint32_t mode);
    int (*rmdir)(const char* path);
    int (*opendir)(const char* path, vfs_file_t** dir);
    int (*readdir)(vfs_file_t* dir, vfs_dirent_t* entry);
    int (*closedir)(vfs_file_t* dir);
    
    // Link operations
    int (*link)(const char* oldpath, const char* newpath);
    int (*unlink)(const char* path);
    int (*symlink)(const char* target, const char* linkpath);
    int (*readlink)(const char* path, char* buffer, size_t buflen);
    
    // File attribute operations
    int (*chmod)(const char* path, uint32_t mode);
    int (*chown)(const char* path, uint32_t uid, uint32_t gid);
    int (*utime)(const char* path, uint64_t atime, uint64_t mtime);
    int (*truncate)(const char* path, off_t length);
    int (*ftruncate)(vfs_file_t* file, off_t length);
    
    // Extended attributes
    ssize_t (*getxattr)(const char* path, const char* name, void* value, size_t size);
    int (*setxattr)(const char* path, const char* name, const void* value, size_t size, int flags);
    ssize_t (*listxattr)(const char* path, char* list, size_t size);
    int (*removexattr)(const char* path, const char* name);
    
    // Mount operations
    int (*mount)(const char* source, const char* target, const char* fstype, 
                 uint32_t flags, const void* data);
    int (*umount)(const char* target);
    int (*umount2)(const char* target, int flags);
    
    // Filesystem registration
    int (*register_filesystem)(vfs_filesystem_t* fs);
    int (*unregister_filesystem)(vfs_filesystem_t* fs);
    
    // Path resolution
    int (*path_lookup)(const char* path, vfs_dentry_t** dentry);
    char* (*getcwd)(char* buffer, size_t size);
    int (*chdir)(const char* path);
    int (*chroot)(const char* path);
    
    // Synchronization
    int (*sync)(void);
    int (*fsync)(vfs_file_t* file);
    int (*fdatasync)(vfs_file_t* file);
    
    // Filesystem information
    int (*statfs)(const char* path, vfs_statfs_t* statfs);
    int (*fstatfs)(vfs_file_t* file, vfs_statfs_t* statfs);
    
    // Advanced features
    int (*splice)(vfs_file_t* in, off_t* off_in, vfs_file_t* out, off_t* off_out, size_t len, uint32_t flags);
    int (*sendfile)(vfs_file_t* out, vfs_file_t* in, off_t* offset, size_t count);
    
    // File locking
    int (*flock)(vfs_file_t* file, int operation);
    int (*fcntl)(vfs_file_t* file, int cmd, void* arg);
    
    // Memory mapping
    int (*mmap_file)(vfs_file_t* file, void** addr, size_t length, int prot, int flags, off_t offset);
} vfs_ops_t;

// Global VFS operations
extern vfs_ops_t* vfs;

// VFS API Functions

// Initialization
int vfs_init(void);
void vfs_cleanup(void);

// File operations
int vfs_open(const char* path, int flags, uint32_t mode, vfs_file_t** file);
int vfs_close(vfs_file_t* file);
ssize_t vfs_read(vfs_file_t* file, void* buffer, size_t count);
ssize_t vfs_write(vfs_file_t* file, const void* buffer, size_t count);
off_t vfs_seek(vfs_file_t* file, off_t offset, int whence);

// Directory operations
int vfs_mkdir(const char* path, uint32_t mode);
int vfs_rmdir(const char* path);
int vfs_opendir(const char* path, vfs_file_t** dir);
int vfs_readdir(vfs_file_t* dir, vfs_dirent_t* entry);

// File information
int vfs_stat(const char* path, vfs_attributes_t* attr);
int vfs_fstat(vfs_file_t* file, vfs_attributes_t* attr);

// Mount operations
int vfs_mount(const char* source, const char* target, const char* fstype, 
              uint32_t flags, const void* data);
int vfs_umount(const char* target);

// Filesystem registration
int vfs_register_filesystem(vfs_filesystem_t* fs);
int vfs_unregister_filesystem(const char* name);

// Path utilities
char* vfs_basename(const char* path);
char* vfs_dirname(const char* path);
int vfs_realpath(const char* path, char* resolved_path);

// RaeenFS specific operations
int raeenfs_create(const char* device, const char* label, uint64_t size);
int raeenfs_check(const char* device, bool repair);
int raeenfs_resize(const char* device, uint64_t new_size);
int raeenfs_defrag(const char* path);
int raeenfs_snapshot(const char* path, const char* snapshot_name);
int raeenfs_clone(const char* source_snapshot, const char* dest);

// Utility macros
#define VFS_ISREG(mode)    (((mode) & VFS_TYPE_REGULAR) == VFS_TYPE_REGULAR)
#define VFS_ISDIR(mode)    (((mode) & VFS_TYPE_DIRECTORY) == VFS_TYPE_DIRECTORY)
#define VFS_ISLNK(mode)    (((mode) & VFS_TYPE_SYMLINK) == VFS_TYPE_SYMLINK)
#define VFS_ISCHR(mode)    (((mode) & VFS_TYPE_CHARDEV) == VFS_TYPE_CHARDEV)
#define VFS_ISBLK(mode)    (((mode) & VFS_TYPE_BLOCKDEV) == VFS_TYPE_BLOCKDEV)
#define VFS_ISFIFO(mode)   (((mode) & VFS_TYPE_FIFO) == VFS_TYPE_FIFO)
#define VFS_ISSOCK(mode)   (((mode) & VFS_TYPE_SOCKET) == VFS_TYPE_SOCKET)

// Permission checking macros
#define VFS_PERM_CHECK(mode, perm) (((mode) & (perm)) == (perm))

// Common file mode combinations
#define VFS_MODE_REG_RW    (VFS_TYPE_REGULAR | VFS_PERM_OWNER_READ | VFS_PERM_OWNER_WRITE)
#define VFS_MODE_DIR_RWX   (VFS_TYPE_DIRECTORY | VFS_PERM_OWNER_READ | VFS_PERM_OWNER_WRITE | VFS_PERM_OWNER_EXEC)

#ifdef __cplusplus
}
#endif

#endif // FILESYSTEM_INTERFACE_H
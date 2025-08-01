// RaeenOS Virtual File System (VFS) Interface
// -------------------------------------------
// This file defines the core, abstract interface for the Virtual File System.
// It provides a unified way for the kernel to interact with various filesystems,
// such as ramfs, fat32, or ext2, without needing to know the underlying details.

#ifndef VFS_H
#define VFS_H

#include "../include/types.h"
#include "../ipc/pipe.h" // For pipe_t

#define VFS_FILENAME_MAX 256

// Flags to identify the type of a VFS node
#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_PIPE        0x03
#define VFS_CHARDEVICE  0x04
#define VFS_BLOCKDEVICE 0x05
#define VFS_SYMLINK     0x06
#define VFS_MOUNTPOINT  0x08 // A directory that is a mountpoint for another fs
#define VFS_FLAG_READABLE 0x10
#define VFS_FLAG_WRITABLE 0x20

struct vfs_node;

// Function pointer types for VFS operations
typedef uint32_t (*vfs_read_t)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef uint32_t (*vfs_write_t)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef void (*vfs_open_t)(struct vfs_node* node, uint32_t flags);
typedef void (*vfs_close_t)(struct vfs_node* node);
typedef struct dirent* (*vfs_readdir_t)(struct vfs_node* node, uint32_t index);
typedef struct vfs_node* (*vfs_finddir_t)(struct vfs_node* node, const char* name);
typedef struct vfs_node* (*vfs_create_t)(struct vfs_node* node, const char* name, uint32_t flags);

// Represents a file, directory, or device in the filesystem tree.
// This is the core abstraction of the VFS.
typedef struct vfs_node {
    char name[VFS_FILENAME_MAX]; // The name of this node
    uint32_t flags;              // Flags (file, directory, etc.)
    uint32_t inode;              // Inode number, unique within a filesystem
    uint32_t length;             // Length of the file in bytes
    uint32_t permissions;        // Access permissions

    // VFS operations
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

    // Filesystem-specific private data
    void* fs_private_data;
} vfs_node_t;

// Represents a directory entry, used by readdir.
struct dirent {
    char name[VFS_FILENAME_MAX];
    uint32_t inode_num;
};

// The root of the filesystem tree
extern vfs_node_t* vfs_root;

// --- VFS API Functions ---

// Initialize the Virtual File System
void vfs_init();

// Find a node in the VFS by its absolute path
vfs_node_t* vfs_find(const char* path);

// Read from a file node
uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);

// Write to a file node
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);

// Create a new file or directory
vfs_node_t* vfs_create(vfs_node_t* parent, const char* name, uint32_t flags);


#endif // VFS_H

// RaeenOS Virtual File System (VFS) Implementation
// ------------------------------------------------
// This file implements the core logic for the VFS layer, managing the
// filesystem tree and dispatching operations to the appropriate drivers.

#include "vfs.h"
#include <stddef.h> // For NULL
#include "../string.h"
#include "ramfs.h"

// The root of the filesystem tree.
// This will be initialized to a ramdisk or a physical disk filesystem.
vfs_node_t* vfs_root = NULL;

// Generic VFS read function. Dispatches to the node's specific read implementation.
uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node) return 0;

    // If it's a pipe, use the pipe reading function (offset is ignored)
    if (node->flags & VFS_PIPE) {
        if (node->pipe) {
            return pipe_read(node->pipe, buffer, size);
        }
        return 0; // Pipe node not properly initialized
    }

    // Otherwise, use the node's standard read function
    if (node->read) {
        return node->read(node, offset, size, buffer);
    }

    return 0; // Error or not supported
}

// Generic VFS write function. Dispatches to the node's specific write implementation.
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node) return 0;

    // If it's a pipe, use the pipe writing function (offset is ignored)
    if (node->flags & VFS_PIPE) {
        if (node->pipe) {
            return pipe_write(node->pipe, buffer, size);
        }
        return 0; // Pipe node not properly initialized
    }

    // Otherwise, use the node's standard write function
    if (node->write) {
        return node->write(node, offset, size, buffer);
    }

    return 0; // Error or not supported
}

// Generic VFS open function.
void vfs_open(vfs_node_t* node, uint32_t flags) {
    if (node && node->open) {
        node->open(node, flags);
    }
}

// Dispatches a close call to the appropriate filesystem driver.
void vfs_close(vfs_node_t* node) {
    if (node && node->close) {
        node->close(node);
    }
}





// Generic VFS readdir function.
struct dirent* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (node && (node->flags & VFS_DIRECTORY) && node->readdir) {
        return node->readdir(node, index);
    }
    return NULL;
}

// Generic VFS finddir function.
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if (node && (node->flags & VFS_DIRECTORY) && node->finddir) {
        return node->finddir(node, name);
    }
    return NULL;
}

// Dispatches a create call to the appropriate filesystem driver.
vfs_node_t* vfs_create(vfs_node_t* parent, const char* name, uint32_t flags) {
    if (parent && parent->create) {
        return parent->create(parent, name, flags);
    }
    return NULL;
}

// Traverses the filesystem to find a node at a given path.
vfs_node_t* vfs_find(const char* path) {
    if (!vfs_root || !path) {
        return NULL;
    }

    // Start from the root directory
    vfs_node_t* current_node = vfs_root;

    // Handle the root path specially
    if (strcmp(path, "/") == 0) {
        return vfs_root;
    }

    // Make a mutable copy of the path for strtok_r
    char path_copy[VFS_FILENAME_MAX];
    strcpy(path_copy, path);

    // Tokenize the path
    char* saveptr;
    char* token = strtok_r(path_copy, "/", &saveptr);

    while (token != NULL) {
        // Ensure the current node is a directory
        if (!(current_node->flags & VFS_DIRECTORY)) {
            return NULL; // Path component is not a directory
        }

        // Find the next node in the current directory
        current_node = vfs_finddir(current_node, token);

        if (current_node == NULL) {
            return NULL; // Path not found
        }

        token = strtok_r(NULL, "/", &saveptr);
    }

    return current_node;
}

// Initializes the VFS by mounting an initial ramdisk as the root.
void vfs_init() {
    vfs_root = ramfs_init();
}

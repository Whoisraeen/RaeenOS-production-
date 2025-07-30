// RaeenOS In-Memory Filesystem (RamFS)
// -------------------------------------
// This file defines the interface for a simple, temporary filesystem
// that stores all its data in RAM. It's primarily used for the initial
// root filesystem before real disk drivers are available.

#ifndef RAMFS_H
#define RAMFS_H

#include "vfs.h"

// Initializes a ramfs and mounts it as the VFS root.
// Returns the root node of the new filesystem.
vfs_node_t* ramfs_init();

/**
 * @brief Creates a special pipe file node in the ramfs.
 * 
 * @param parent The directory to create the pipe in.
 * @param name The name of the pipe file.
 * @return vfs_node_t* A pointer to the newly created pipe node, or NULL on failure.
 */
vfs_node_t* ramfs_create_pipe(vfs_node_t* parent, const char* name);

#endif // RAMFS_H

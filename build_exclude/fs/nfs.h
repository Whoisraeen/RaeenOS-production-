#ifndef NFS_H
#define NFS_H

#include "../include/types.h"
#include "vfs.h"

// NFS mount function (placeholder)
vfs_node_t* nfs_mount(const char* server_address, const char* remote_path);

#endif // NFS_H

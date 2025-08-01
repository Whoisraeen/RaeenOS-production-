// RaeenOS In-Memory Filesystem (RamFS) Implementation

#include "ramfs.h"
#include "../string.h"

// Using types.h for kernel build
#include "../pmm.h"
#include "../ipc/pipe.h"

#define MAX_RAMFS_NODES 256
#define MAX_DIR_ENTRIES 16

typedef struct {
    union {
        vfs_node_t* children[MAX_DIR_ENTRIES];
        uint8_t* file_data;
    };
} ramfs_data_t;

static vfs_node_t ramfs_nodes[MAX_RAMFS_NODES];
static ramfs_data_t ramfs_node_data[MAX_RAMFS_NODES];
static struct dirent ramfs_dirent_buffer; // A single, reusable buffer for readdir
static uint32_t next_node_index = 0;

// Forward declarations for our static VFS functions
static uint32_t ramfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
static uint32_t ramfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
static struct dirent* ramfs_readdir(vfs_node_t* node, uint32_t index);
static vfs_node_t* ramfs_finddir(vfs_node_t* node, const char* name);
static vfs_node_t* ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags);
static vfs_node_t* ramfs_alloc_node(const char* name, uint32_t flags);

vfs_node_t* ramfs_init() {
    vfs_node_t* root = ramfs_alloc_node("/", VFS_DIRECTORY);
    return root;
}

static vfs_node_t* ramfs_alloc_node(const char* name, uint32_t flags) {
    if (next_node_index >= MAX_RAMFS_NODES) return NULL;

    uint32_t i = next_node_index++;
    vfs_node_t* node = &ramfs_nodes[i];
    ramfs_data_t* data = &ramfs_node_data[i];

    strcpy(node->name, name);
    node->inode = i;
    node->flags = flags;
    node->length = 0;
        node->mounted_at = NULL;
    node->pipe = NULL;
    node->open = NULL; 
    node->close = NULL;

    if (flags & VFS_DIRECTORY) {
        node->read = NULL;
        node->write = NULL;
        node->readdir = ramfs_readdir;
        node->finddir = ramfs_finddir;
        node->create = ramfs_create;
        for (int j = 0; j < MAX_DIR_ENTRIES; j++) data->children[j] = NULL;
    } else {
        node->read = ramfs_read;
        node->write = ramfs_write;
        node->readdir = NULL;
        node->finddir = NULL;
        node->create = NULL;
        data->file_data = NULL;
    }
    return node;
}

static uint32_t ramfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->flags & VFS_DIRECTORY) return 0;
    ramfs_data_t* data = &ramfs_node_data[node->inode];
    if (!data->file_data || offset > node->length) return 0;
    if (offset + size > node->length) size = node->length - offset;

    memcpy(buffer, data->file_data + offset, size);
    return size;
}

static uint32_t ramfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->flags & VFS_DIRECTORY) return 0;
    ramfs_data_t* data = &ramfs_node_data[node->inode];
    if (!data->file_data) {
        data->file_data = (uint8_t*)pmm_alloc_frame();
        if (!data->file_data) return 0; 
    }

    if (offset + size > PMM_FRAME_SIZE) size = PMM_FRAME_SIZE - offset;
    if (offset > PMM_FRAME_SIZE) return 0;

    memcpy(data->file_data + offset, buffer, size);
    if (offset + size > node->length) node->length = offset + size;
    return size;
}

static struct dirent* ramfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!(node->flags & VFS_DIRECTORY)) return NULL;
    ramfs_data_t* data = &ramfs_node_data[node->inode];
    
    if (index < MAX_DIR_ENTRIES && data->children[index]) {
        strcpy(ramfs_dirent_buffer.name, data->children[index]->name);
        ramfs_dirent_buffer.inode_num = data->children[index]->inode;
        return &ramfs_dirent_buffer;
    }
    return NULL;
}

static vfs_node_t* ramfs_finddir(vfs_node_t* node, const char* name) {
    if (!(node->flags & VFS_DIRECTORY)) return NULL;
    ramfs_data_t* data = &ramfs_node_data[node->inode];
    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (data->children[i] && strcmp(data->children[i]->name, name) == 0) {
            return data->children[i];
        }
    }
    return NULL;
}

vfs_node_t* ramfs_create_pipe(vfs_node_t* parent, const char* name) {
    if (!(parent->flags & VFS_DIRECTORY)) return NULL;
    ramfs_data_t* data = &ramfs_node_data[parent->inode];

    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (!data->children[i]) {
            // Allocate a VFS node with the pipe flag
            vfs_node_t* new_node = ramfs_alloc_node(name, VFS_PIPE);
            if (!new_node) {
                return NULL; // Allocation failed
            }

            // Create the underlying pipe object
            new_node->pipe = pipe_create();
            if (!new_node->pipe) {
                // In a more robust implementation, we would free the allocated vfs_node here.
                // For ramfs, we have a fixed pool, so we just fail.
                return NULL;
            }
            
            // Link it into the directory
            data->children[i] = new_node;
            return new_node;
        }
    }

    return NULL; // Directory is full
}

static vfs_node_t* ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags) {
    if (!(parent->flags & VFS_DIRECTORY)) return NULL;
    ramfs_data_t* data = &ramfs_node_data[parent->inode];
    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (!data->children[i]) {
            vfs_node_t* new_node = ramfs_alloc_node(name, flags);
            if (new_node) data->children[i] = new_node;
            return new_node;
        }
    }
    return NULL;
}

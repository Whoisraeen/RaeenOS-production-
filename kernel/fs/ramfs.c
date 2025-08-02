#include "ramfs.h"
#include "../memory.h"
#include "../string.h"
#include "../vga.h"

// Simple RAMFS node structure
typedef struct ramfs_node {
    vfs_node_t vfs_node; // Inherit from VFS node
    uint8_t* data;
    size_t size;
    size_t capacity;
    struct ramfs_node* children;
    struct ramfs_node* next_sibling;
} ramfs_node_t;

static ramfs_node_t* ramfs_root = NULL;

// VFS operations for RAMFS
static ssize_t ramfs_read(vfs_node_t* node, void* buffer, size_t size, off_t offset);
static ssize_t ramfs_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset);
static vfs_node_t* ramfs_finddir(vfs_node_t* node, const char* name);
static vfs_node_t* ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags);
static int ramfs_mkdir(vfs_node_t* parent, const char* name, uint32_t flags);

void ramfs_init(void) {
    debug_print("Initializing RAMFS...\n");

    ramfs_root = (ramfs_node_t*)kmalloc(sizeof(ramfs_node_t));
    if (!ramfs_root) {
        debug_print("RAMFS: Failed to allocate root node!\n");
        return;
    }

    memset(ramfs_root, 0, sizeof(ramfs_node_t));
    strncpy(ramfs_root->vfs_node.name, "/", sizeof(ramfs_root->vfs_node.name) - 1);
    ramfs_root->vfs_node.flags = VFS_DIRECTORY;
    ramfs_root->vfs_node.mask = 0777; // rwxrwxrwx
    ramfs_root->vfs_node.inode = 0;
    ramfs_root->vfs_node.length = 0;
    ramfs_root->vfs_node.impl = ramfs_root; // Point back to itself

    // Assign VFS operations
    ramfs_root->vfs_node.read = ramfs_read;
    ramfs_root->vfs_node.write = ramfs_write;
    ramfs_root->vfs_node.finddir = ramfs_finddir;
    ramfs_root->vfs_node.create = ramfs_create;
    ramfs_root->vfs_node.mkdir = ramfs_mkdir;

    debug_print("RAMFS initialized. Root node created.\n");
}

int ramfs_mount(const char* path) {
    if (!ramfs_root) {
        debug_print("RAMFS not initialized.\n");
        return -1;
    }
    if (vfs_mount(path, ramfs_root) != 0) {
        debug_print("RAMFS: Failed to mount at ");
        debug_print(path);
        debug_print("\n");
        return -1;
    }
    debug_print("RAMFS mounted at ");
    debug_print(path);
    debug_print("\n");
    return 0;
}

static ssize_t ramfs_read(vfs_node_t* node, void* buffer, size_t size, off_t offset) {
    ramfs_node_t* rnode = (ramfs_node_t*)node->impl;
    if (!rnode || !(node->flags & VFS_FILE)) return -1;

    if (offset >= rnode->size) return 0; // EOF
    if (offset + size > rnode->size) size = rnode->size - offset;

    memcpy(buffer, rnode->data + offset, size);
    return size;
}

static ssize_t ramfs_write(vfs_node_t* node, const void* buffer, size_t size, off_t offset) {
    ramfs_node_t* rnode = (ramfs_node_t*)node->impl;
    if (!rnode || !(node->flags & VFS_FILE)) return -1;

    // Expand capacity if needed
    if (offset + size > rnode->capacity) {
        size_t new_capacity = rnode->capacity == 0 ? 4096 : rnode->capacity * 2;
        while (offset + size > new_capacity) new_capacity *= 2;

        uint8_t* new_data = (uint8_t*)kmalloc(new_capacity);
        if (!new_data) return -1; // Out of memory

        if (rnode->data) {
            memcpy(new_data, rnode->data, rnode->size);
            kfree(rnode->data);
        }
        rnode->data = new_data;
        rnode->capacity = new_capacity;
    }

    memcpy(rnode->data + offset, buffer, size);
    if (offset + size > rnode->size) rnode->size = offset + size;
    node->length = rnode->size;

    return size;
}

static vfs_node_t* ramfs_finddir(vfs_node_t* node, const char* name) {
    ramfs_node_t* rnode = (ramfs_node_t*)node->impl;
    if (!rnode || !(node->flags & VFS_DIRECTORY)) return NULL;

    ramfs_node_t* child = rnode->children;
    while (child) {
        if (strcmp(child->vfs_node.name, name) == 0) {
            return &child->vfs_node;
        }
        child = child->next_sibling;
    }
    return NULL;
}

static vfs_node_t* ramfs_create(vfs_node_t* parent, const char* name, uint32_t flags) {
    ramfs_node_t* rparent = (ramfs_node_t*)parent->impl;
    if (!rparent || !(parent->flags & VFS_DIRECTORY)) return NULL;

    ramfs_node_t* new_node = (ramfs_node_t*)kmalloc(sizeof(ramfs_node_t));
    if (!new_node) return NULL;

    memset(new_node, 0, sizeof(ramfs_node_t));
    strncpy(new_node->vfs_node.name, name, sizeof(new_node->vfs_node.name) - 1);
    new_node->vfs_node.flags = flags | VFS_FILE; // Always a file for create
    new_node->vfs_node.mask = 0666; // rw-rw-rw-
    new_node->vfs_node.inode = 0; // TODO: Assign unique inode
    new_node->vfs_node.length = 0;
    new_node->vfs_node.impl = new_node;

    new_node->vfs_node.read = ramfs_read;
    new_node->vfs_node.write = ramfs_write;

    // Add to parent's children list
    new_node->next_sibling = rparent->children;
    rparent->children = new_node;

    debug_print("RAMFS: Created file ");
    debug_print(name);
    debug_print("\n");

    return &new_node->vfs_node;
}

static int ramfs_mkdir(vfs_node_t* parent, const char* name, uint32_t flags) {
    ramfs_node_t* rparent = (ramfs_node_t*)parent->impl;
    if (!rparent || !(parent->flags & VFS_DIRECTORY)) return -1;

    ramfs_node_t* new_node = (ramfs_node_t*)kmalloc(sizeof(ramfs_node_t));
    if (!new_node) return -1;

    memset(new_node, 0, sizeof(ramfs_node_t));
    strncpy(new_node->vfs_node.name, name, sizeof(new_node->vfs_node.name) - 1);
    new_node->vfs_node.flags = flags | VFS_DIRECTORY;
    new_node->vfs_node.mask = 0777; // rwxrwxrwx
    new_node->vfs_node.inode = 0; // TODO: Assign unique inode
    new_node->vfs_node.length = 0;
    new_node->vfs_node.impl = new_node;

    new_node->vfs_node.finddir = ramfs_finddir;
    new_node->vfs_node.create = ramfs_create;
    new_node->vfs_node.mkdir = ramfs_mkdir;

    // Add to parent's children list
    new_node->next_sibling = rparent->children;
    rparent->children = new_node;

    debug_print("RAMFS: Created directory ");
    debug_print(name);
    debug_print("\n");

    return 0;
}
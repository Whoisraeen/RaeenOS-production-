/**
 * @file vfs.h
 * @brief RaeenOS Unified Virtual File System Interface
 * 
 * This is the unified VFS interface that provides both legacy compatibility
 * and modern production-grade features. This header resolves the conflicts
 * between the old basic VFS and the production VFS by using the production
 * implementation as the foundation while maintaining backward compatibility.
 * 
 * Architecture Decision: Use vfs_production.h as the unified interface
 * - Production VFS already includes legacy compatibility structures
 * - Provides enterprise-grade features and performance
 * - Resolves memory interface conflicts
 * - Maintains backward compatibility for existing code
 * 
 * Version: 3.0 - Unified Architecture
 * Replaces: vfs.h (basic) and vfs_production.h (production)
 */

#ifndef VFS_H
#define VFS_H

// Include the production VFS interface which contains all functionality
#include "vfs_production.h"

// Additional legacy compatibility if needed
// The production VFS already includes all legacy structures and functions

// Memory interface compatibility - resolve conflicts
// Don't define these macros if memory_interface.h is already included
#ifndef MEMORY_INTERFACE_H
// Legacy memory function declarations (avoid macro conflicts)
extern void* kmalloc_compat(size_t size);
extern void memory_init_compat(void);
#endif

// Legacy API compatibility declarations
// These functions bridge the legacy VFS interface to the production implementation
extern void vfs_init_legacy(void);
extern void vfs_open_legacy(vfs_node_t* node, uint32_t flags);
extern void vfs_close_legacy(vfs_node_t* node);

// Legacy API compatibility - the production VFS already defines:
// struct vfs_node -> defined in vfs_production.h (lines 516-544) with legacy compatibility
// struct dirent -> defined in vfs_production.h (lines 509-513) with legacy compatibility
// vfs_init() -> production version returns int, use vfs_init_legacy() for void version
// vfs_find() -> defined in vfs_production.h (line 720) for legacy compatibility
// vfs_read() -> defined in vfs_production.h (line 721) for legacy compatibility  
// vfs_write() -> defined in vfs_production.h (line 722) for legacy compatibility
// vfs_create() -> defined in vfs_production.h (line 723) for legacy compatibility

#endif // VFS_H
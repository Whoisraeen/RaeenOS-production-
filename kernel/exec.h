#ifndef EXEC_H
#define EXEC_H

#include "include/types.h"
#include "process/process.h"
#include "paging.h"

#define USER_STACK_TOP    0xC0000000
#define USER_STACK_SIZE   0x4000 // 16 KB

#define RAEEXEC_MAGIC 0x5241454E // "RAEN"

// RaeenOS Executable Header
typedef struct {
    uint32_t magic;          // Magic number to identify the file type
    uint32_t entry;          // Virtual address of the entry point
    uint32_t ph_offset;      // Offset of the program header table in the file
    uint16_t ph_entry_size;  // Size of a single program header entry
    uint16_t ph_num;         // Number of entries in the program header table
} raeexec_header_t;

// Program Header (describes a segment to be loaded)
typedef struct {
    uint32_t type;           // Segment type (e.g., PT_LOAD)
    uint32_t offset;         // Offset of the segment in the file
    uint32_t vaddr;          // Virtual address to load the segment
    uint32_t file_size;      // Size of the segment in the file
    uint32_t mem_size;       // Size of the segment in memory (can be > file_size for .bss)
    uint32_t flags;          // Segment flags (e.g., R/W/X)
} raeexec_pheader_t;

// Segment Types
#define PT_NULL    0 // Unused
#define PT_LOAD    1 // Loadable segment

// Segment Flags
#define PF_X 1 // Execute
#define PF_W 2 // Write
#define PF_R 4 // Read

// Forward declarations to avoid circular dependencies
struct process_s;
struct page_directory_s;

/**
 * @brief Loads an executable from a file and creates a new process for it.
 * 
 * @param path The path to the executable file in the VFS.
 * @param argc The number of arguments.
 * @param argv The argument vector.
 * @return A pointer to the new process_t, or NULL on failure.
 */
process_t* exec_load(const char* path, int argc, char** argv);

/**
 * @brief Loads an executable into an existing address space, replacing its content.
 * 
 * @param path The path to the executable file.
 * @param page_dir The page directory of the address space to load into.
 * @return The virtual address of the new entry point, or 0 on failure.
 */
uint32_t exec_load_into_address_space(const char* path, page_directory_t* page_dir);

#endif // EXEC_H

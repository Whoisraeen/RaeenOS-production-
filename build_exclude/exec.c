#include "exec.h"
#include "fs/vfs.h"
#include "process/process.h"
#include "pmm.h"
#include "paging.h"
#include "exec.h"
#include <stdbool.h>
#include "string.h"


#define USER_STACK_SIZE 0x4000 // 16 KB



/**
 * @brief Loads and executes a program from the filesystem.
 * 
 * @param path The path to the executable file.
 * @param argc The number of arguments.
 * @param argv The argument vector.
 * @return A pointer to the new process, or NULL on failure.
 */
uint32_t exec_load_into_address_space(const char* path, page_directory_t* page_dir) {
    // 1. Open the executable file
    vfs_node_t* file = vfs_find((char*)path);
    if (!file) {
        // kprintf("exec: file not found: %s\n", path);
        return 0;
    }

    // 2. Read the header
    raeexec_header_t header;
    if (vfs_read(file, 0, sizeof(header), (uint8_t*)&header) != sizeof(header)) {
        // kprintf("exec: failed to read header\n");
        return 0;
    }

    // 3. Validate the magic number
    if (header.magic != RAEEXEC_MAGIC) {
        // kprintf("exec: invalid magic number\n");
        return 0;
    }

    // 4. Load program segments (pheaders)
    for (int i = 0; i < header.ph_num; ++i) {
        raeexec_pheader_t pheader;
        uint32_t offset = header.ph_offset + i * header.ph_entry_size;
        if (vfs_read(file, offset, sizeof(pheader), (uint8_t*)&pheader) != sizeof(pheader)) {
            // kprintf("exec: failed to read pheader\n");
            return 0;
        }

        if (pheader.type == PT_LOAD) {
            // Allocate physical memory for the segment
            for (uint32_t j = 0; j < pheader.mem_size; j += PAGE_SIZE) {
                void* p_addr = pmm_alloc_frame();
                if (!p_addr) {
                    // TODO: Clean up everything allocated so far
                    return 0;
                }
                paging_map_page(page_dir, (void*)(pheader.vaddr + j), p_addr, true, true);
            }

            // Now copy the data from the file into the new address space
            // We need to switch to the new page directory to write to the virtual address
            page_directory_t* old_dir = paging_get_current_directory();
            paging_switch_directory(page_dir);

            vfs_read(file, pheader.offset, pheader.file_size, (uint8_t*)pheader.vaddr);
            // Zero out the .bss section if necessary
            if (pheader.mem_size > pheader.file_size) {
                memset((void*)(pheader.vaddr + pheader.file_size), 0, pheader.mem_size - pheader.file_size);
            }

            paging_switch_directory(old_dir);
        }
    }

    return header.entry;
}

process_t* exec_load(const char* path, int argc, char** argv) {
    // 1. Create a new address space for the process
    page_directory_t* page_dir = paging_create_address_space();
    if (!page_dir) {
        return NULL; // Failed to create address space
    }

    // 2. Load the executable into the new address space
    uint32_t entry_point = exec_load_into_address_space(path, page_dir);
    if (entry_point == 0) {
        // TODO: Free the created page directory and its frames
        return NULL; // Failed to load executable
    }

    // 3. Set up the user stack
    for (uint32_t addr = USER_STACK_TOP - USER_STACK_SIZE; addr < USER_STACK_TOP; addr += PAGE_SIZE) {
        void* p_addr = pmm_alloc_frame();
        if (!p_addr) {
            // TODO: Clean up
            return NULL;
        }
        paging_map_page(page_dir, (void*)addr, p_addr, true, true);
    }

    // 4. Create the process
    process_t* new_proc = process_create((void (*)(void))entry_point);
    if (!new_proc) {
        // TODO: Clean up page directory and frames
        return NULL;
    }

    new_proc->page_directory = page_dir;

    // TODO: Push argc and argv to the user stack

    return new_proc;
}

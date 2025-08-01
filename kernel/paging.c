// RaeenOS Paging (Virtual Memory Manager)

#include "paging.h"
#include "pmm.h"
#include "idt.h"
#include "vga.h"
#include "string.h"

extern uint32_t highest_address;

// The kernel's page directory
// We will allocate this on the stack for now, but in a real OS it should be in a known physical location.
page_directory_t* kernel_directory = 0;

// External function to load the page directory and enable paging
extern void load_page_directory(page_directory_t*);
extern void enable_paging(void);

// Page fault handler
void page_fault_handler(struct registers_t regs) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us details of what happened.
    int present   = !(regs.err_code & 0x1); // Page not present
    int rw = regs.err_code & 0x2;           // Write operation?
    int us = regs.err_code & 0x4;           // User-mode?
    int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

    vga_puts("\nPage Fault! (");
    if (present) vga_puts("present ");
    if (rw) vga_puts("read-only ");
    if (us) vga_puts("user-mode ");
    if (reserved) vga_puts("reserved ");
    if (id) vga_puts("instruction-fetch ");
    vga_puts(") at 0x");
    // Simple hex print for now
    char hex[9];
    const char* h = "0123456789ABCDEF";
    hex[8] = 0;
    for(int i = 7; i >= 0; --i) {
        hex[i] = h[faulting_address & 0xF];
        faulting_address >>= 4;
    }
    vga_puts(hex);
    vga_puts("\n");

    // For now, we just halt.
    vga_puts("System Halted.\n");
    asm volatile ("cli; hlt");
}

void paging_init(void) {
    // Allocate the kernel's page directory
    kernel_directory = (page_directory_t*)pmm_alloc_frame();
    if (!kernel_directory) {
        // PANIC! Cannot allocate frame for page directory.
        return;
    }
    memset(kernel_directory, 0, sizeof(page_directory_t));

    // Identity-map all physical memory
    for (uint32_t addr = 0; addr < highest_address; addr += PAGE_SIZE) {
        paging_map_page(kernel_directory, (void*)addr, (void*)addr, false, true);
    }

    // Register our page fault handler (interrupt 14)
    register_interrupt_handler(14, (isr_t)page_fault_handler);

    // Load the new page directory
    paging_switch_directory(kernel_directory);

    enable_paging();
}

// Returns a pointer to the kernel's page directory.
page_directory_t* paging_get_kernel_directory(void) {
    return kernel_directory;
}

page_directory_t* paging_create_address_space(void) {
    // Allocate a new page directory
    page_directory_t* dir = (page_directory_t*)pmm_alloc_frame();
    if (!dir) {
        return NULL;
    }

    // Clear the new directory and copy kernel-space mappings
    // A common convention is to map the kernel in the upper part of the address space.
    // For now, we'll assume the kernel occupies the first 4MB, so we copy that mapping.
    memset(dir, 0, sizeof(page_directory_t));
    dir->entries[0] = kernel_directory->entries[0];

    return dir;
}

void paging_map_page(page_directory_t* dir, void* virt, void* phys, bool is_user, bool is_writable) {
    uintptr_t virt_addr = (uintptr_t)virt;
    uint32_t dir_idx = virt_addr / (1024 * PAGE_SIZE);
    uint32_t table_idx = (virt_addr / PAGE_SIZE) % 1024;

    // Get the page table, creating it if it doesn't exist
    page_table_t* table;
    if (!(dir->entries[dir_idx] & PDE_PRESENT)) {
        table = (page_table_t*)pmm_alloc_frame();
        if (!table) {
            // PANIC: Out of memory
            return;
        }
        memset(table, 0, sizeof(page_table_t));
        dir->entries[dir_idx] = (uintptr_t)table | PDE_PRESENT | PDE_READ_WRITE | (is_user ? PDE_USER : 0);
    } else {
        table = (page_table_t*)(dir->entries[dir_idx] & ~0xFFF);
    }

    // Set the page table entry
    uint32_t flags = PTE_PRESENT;
    if (is_writable) flags |= PTE_READ_WRITE;
    if (is_user) flags |= PTE_USER;
    table->entries[table_idx] = (uintptr_t)phys | flags;
}

void paging_map_kernel_page(page_directory_t* dir, void* virt, void* phys, bool is_writable) {
    uintptr_t virt_addr = (uintptr_t)virt;
    uint32_t dir_idx = virt_addr / (1024 * PAGE_SIZE);
    uint32_t table_idx = (virt_addr / PAGE_SIZE) % 1024;

    // Get the page table, creating it if it doesn't exist
    page_table_t* table;
    if (!(dir->entries[dir_idx] & PDE_PRESENT)) {
        table = (page_table_t*)pmm_alloc_frame();
        if (!table) {
            // PANIC: Out of memory
            return;
        }
        memset(table, 0, sizeof(page_table_t));
        dir->entries[dir_idx] = (uintptr_t)table | PDE_PRESENT | PDE_READ_WRITE; // Kernel pages are always writable by kernel
    } else {
        table = (page_table_t*)(dir->entries[dir_idx] & ~0xFFF);
    }

    // Set the page table entry (always supervisor, present)
    uint32_t flags = PTE_PRESENT;
    if (is_writable) flags |= PTE_READ_WRITE;
    table->entries[table_idx] = (uintptr_t)phys | flags;
}

void paging_switch_directory(page_directory_t* dir) {
    asm volatile("mov %0, %%cr3" :: "r"(dir));
}

page_directory_t* paging_get_current_directory(void) {
    page_directory_t* dir;
    asm volatile("mov %%cr3, %0" : "=r"(dir));
    return dir;
}

// A helper function to map a temporary page for copying
static void* temp_map_page(void* phys_addr) {
    // This is a simplified and potentially unsafe way to get a temporary virtual address.
    // A more robust implementation would manage a dedicated region for temporary mappings.
    // We'll use a high virtual address that is unlikely to be in use.
    const uintptr_t temp_vaddr = 0xFFC00000;
    page_directory_t* kernel_dir = paging_get_kernel_directory();
    paging_map_page(kernel_dir, (void*)temp_vaddr, phys_addr, false, true);
    return (void*)temp_vaddr;
}

// A helper function to unmap the temporary page
static void temp_unmap_page(void) {
    const uintptr_t temp_vaddr = 0xFFC00000;
    page_directory_t* kernel_dir = paging_get_kernel_directory();
    uint32_t dir_idx = temp_vaddr / (1024 * PAGE_SIZE);
    uint32_t table_idx = (temp_vaddr / PAGE_SIZE) % 1024;

    page_table_t* table = (page_table_t*)(kernel_dir->entries[dir_idx] & ~0xFFF);
    table->entries[table_idx] = 0;
    
    // Invalidate the TLB entry for our temporary page
    asm volatile("invlpg (%0)" :: "r"(temp_vaddr) : "memory");
}

page_directory_t* paging_clone_directory(page_directory_t* src) {
    page_directory_t* new_dir = (page_directory_t*)pmm_alloc_frame();
    if (!new_dir) return NULL;
    memset(new_dir, 0, sizeof(page_directory_t));

    // Copy kernel space mappings (link, don't copy)
    for (int i = 768; i < 1024; ++i) {
        if (src->entries[i] & PDE_PRESENT) {
            new_dir->entries[i] = src->entries[i];
        }
    }

    // Copy user space mappings (deep copy)
    for (int i = 0; i < 768; ++i) {
        if (src->entries[i] & PDE_PRESENT) {
            page_table_t* parent_table = (page_table_t*)(src->entries[i] & ~0xFFF);
            page_table_t* child_table = (page_table_t*)pmm_alloc_frame();
            if (!child_table) {
                // TODO: Clean up previously allocated memory
                return NULL;
            }
            memset(child_table, 0, sizeof(page_table_t));

            for (int j = 0; j < 1024; ++j) {
                if (parent_table->entries[j] & PTE_PRESENT) {
                    void* parent_phys_addr = (void*)(parent_table->entries[j] & ~0xFFF);
                    void* child_phys_addr = pmm_alloc_frame();
                    if (!child_phys_addr) {
                        // TODO: Clean up
                        return NULL;
                    }

                    // Map parent and child pages to copy data
                    uintptr_t virt_addr = (i * 1024 + j) * PAGE_SIZE;
                    void* temp_child_vaddr = temp_map_page(child_phys_addr);
                    memcpy(temp_child_vaddr, (void*)virt_addr, PAGE_SIZE);
                    temp_unmap_page();
                    
                    // Add entry to child's page table
                    uint32_t flags = parent_table->entries[j] & 0xFFF;
                    child_table->entries[j] = (uintptr_t)child_phys_addr | flags;
                }
            }
            new_dir->entries[i] = (uintptr_t)child_table | (src->entries[i] & 0xFFF);
        }
    }
    return new_dir;
}

void paging_free_directory(page_directory_t* dir) {
    if (!dir) {
        return;
    }

    for (int i = 0; i < 768; ++i) { // Iterate over user-space entries only
        if (dir->entries[i] & PDE_PRESENT) {
            page_table_t* table = (page_table_t*)(dir->entries[i] & ~0xFFF);
            
            for (int j = 0; j < 1024; ++j) {
                if (table->entries[j] & PTE_PRESENT) {
                    // Free the physical frame mapped by this page
                    void* frame_addr = (void*)(table->entries[j] & ~0xFFF);
                    pmm_free_frame(frame_addr);
                }
            }

            // Free the physical frame occupied by the page table itself
            pmm_free_frame(table);
        }
    }

    // Free the physical frame occupied by the page directory itself
    pmm_free_frame(dir);
}

void paging_free_user_pages(page_directory_t* dir) {
    if (!dir) {
        return;
    }

    // Free all user-space page tables and their mapped pages
    for (int i = 0; i < 768; ++i) { // Iterate over user-space entries only
        if (dir->entries[i] & PDE_PRESENT) {
            page_table_t* table = (page_table_t*)(dir->entries[i] & ~0xFFF);
            
            for (int j = 0; j < 1024; ++j) {
                if (table->entries[j] & PTE_PRESENT) {
                    // Free the physical frame mapped by this page
                    void* frame_addr = (void*)(table->entries[j] & ~0xFFF);
                    pmm_free_frame(frame_addr);
                    table->entries[j] = 0; // Clear the entry
                }
            }

            // Free the physical frame occupied by the page table itself
            pmm_free_frame(table);
            dir->entries[i] = 0; // Clear the directory entry
        }
    }
}

bool is_valid_userspace_ptr(void* addr, size_t size) {
    uintptr_t start_addr = (uintptr_t)addr;
    uintptr_t end_addr = start_addr + size;

    // Check if the address range is within user space (0 to 0xC0000000 - 1)
    if (start_addr >= 0xC0000000 || end_addr > 0xC0000000 || start_addr > end_addr) {
        return false;
    }

    // Check if all pages in the range are present and user-accessible
    page_directory_t* current_dir = paging_get_current_directory();

    for (uintptr_t current_virt_addr = start_addr; current_virt_addr < end_addr; current_virt_addr += PAGE_SIZE) {
        uint32_t dir_idx = current_virt_addr / (1024 * PAGE_SIZE);
        uint32_t table_idx = (current_virt_addr / PAGE_SIZE) % 1024;

        if (!(current_dir->entries[dir_idx] & PDE_PRESENT)) {
            return false; // Page directory entry not present
        }

        page_table_t* table = (page_table_t*)(current_dir->entries[dir_idx] & ~0xFFF);

        if (!(table->entries[table_idx] & PTE_PRESENT)) {
            return false; // Page table entry not present
        }

        if (!(table->entries[table_idx] & PTE_USER)) {
            return false; // Not user-accessible
        }
    }

    return true;
}

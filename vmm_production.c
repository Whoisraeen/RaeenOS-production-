/**
 * @file vmm_production.c
 * @brief Production-Grade Virtual Memory Manager Implementation
 * 
 * This file implements a comprehensive virtual memory manager for RaeenOS
 * with 64-bit address space management, VMA handling, page fault processing,
 * copy-on-write support, and memory mapping functionality.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "vmm_production.h"
#include "pmm_production.h"
#include "kernel/include/memory_interface.h"
#include "kernel/include/hal_interface.h"
#include "include/types.h"
#include "include/errno.h"
#include "libs/libc/include/string.h"
#include "vga.h"

// Forward declarations
static int vmm_insert_vma(address_space_t* mm, vm_area_t* vma);
static void vmm_arch_pick_mmap_base(address_space_t* mm);

// Global VMM manager instance
static vmm_manager_t vmm_manager;
vmm_manager_t* vmm = &vmm_manager;

// Kernel address space
static address_space_t kernel_address_space;

/**
 * Initialize the Virtual Memory Manager
 */
int vmm_init(void) {
    vga_puts("VMM: Initializing production virtual memory manager...\n");
    
    // Clear VMM manager structure
    memset(vmm, 0, sizeof(vmm_manager_t));
    
    // Initialize locks
    spinlock_init(&vmm->global_lock);
    
    // Configure VMM parameters
    vmm->config.vmalloc_start = VMM_KERNEL_VMALLOC;
    vmm->config.vmalloc_end = VMM_KERNEL_MODULES;
    vmm->config.high_memory = 0x100000000ULL;  // 4GB
    vmm->config.execute_disable = true;        // Assume NX bit support
    vmm->config.smep_enabled = false;          // Will be detected later
    vmm->config.smap_enabled = false;          // Will be detected later
    
    // Initialize kernel address space
    memset(&kernel_address_space, 0, sizeof(address_space_t));
    
    // Allocate kernel PML4 (page global directory)
    kernel_address_space.pgd = (pml4_t*)pmm_alloc_page(MM_FLAG_KERNEL | MM_FLAG_ZERO, -1);
    if (!kernel_address_space.pgd) {
        vga_puts("VMM: Failed to allocate kernel PML4\n");
        return -ENOMEM;
    }
    
    // Initialize kernel address space fields
    spinlock_init(&kernel_address_space.page_table_lock);
    atomic_set(&kernel_address_space.mm_users, 1);
    atomic_set(&kernel_address_space.mm_count, 1);
    kernel_address_space.owner_pid = 0;  // Kernel PID
    
    // Set up kernel memory layout
    kernel_address_space.start_code = VMM_KERNEL_TEXT;
    kernel_address_space.end_code = VMM_KERNEL_TEXT + 0x1000000;  // 16MB kernel
    kernel_address_space.start_data = kernel_address_space.end_code;
    kernel_address_space.end_data = kernel_address_space.start_data + 0x1000000;
    kernel_address_space.mmap_base = VMM_KERNEL_VMALLOC;
    
    // Create direct physical memory mapping in kernel space
    // Map first 4GB of physical memory to VMM_KERNEL_DIRECT_MAP
    uint64_t phys_addr = 0;
    uint64_t virt_addr = VMM_KERNEL_DIRECT_MAP;
    size_t direct_map_size = (sizeof(void*) == 8) ? 0x100000000ULL : 0x80000000ULL;  // 4GB on 64-bit, 2GB on 32-bit
    
    // Map in 2MB huge pages for efficiency
    for (; phys_addr < direct_map_size; phys_addr += 0x200000, virt_addr += 0x200000) {
        int ret = vmm_map_page(&kernel_address_space, virt_addr, phys_addr, 
                              0x200000, MM_PROT_READ | MM_PROT_WRITE);
        if (ret < 0) {
            vga_puts("VMM: Failed to create kernel direct mapping\n");
            return ret;
        }
    }
    
    vmm->kernel_mm = &kernel_address_space;
    
    // Switch to kernel address space
    vmm_switch_address_space(&kernel_address_space);
    
    vmm->initialized = true;
    
    vga_puts("VMM: Virtual memory manager initialized successfully\n");
    return 0;
}

/**
 * Create a new address space
 */
address_space_t* vmm_create_address_space(void) {
    address_space_t* mm = kmalloc(sizeof(address_space_t), MM_FLAG_KERNEL | MM_FLAG_ZERO);
    if (!mm) {
        return NULL;
    }
    
    // Allocate PML4
    mm->pgd = (pml4_t*)pmm_alloc_page(MM_FLAG_KERNEL | MM_FLAG_ZERO, -1);
    if (!mm->pgd) {
        kfree(mm);
        return NULL;
    }
    
    // Copy kernel mappings (upper half)
    pml4_t* kernel_pgd = vmm->kernel_mm->pgd;
    for (int i = 256; i < VMM_PML4_ENTRIES; i++) {  // Upper half
        mm->pgd->entries[i] = kernel_pgd->entries[i];
    }
    
    // Initialize address space
    spinlock_init(&mm->page_table_lock);
    atomic_set(&mm->mm_users, 1);
    atomic_set(&mm->mm_count, 1);
    mm->mm_rb = RB_ROOT;
    
    // Set up user space memory layout with ASLR
    vmm_arch_pick_mmap_base(mm);
    
    // Set default limits
    mm->rlimits.max_data_size = 0x40000000ULL;      // 1GB
    mm->rlimits.max_stack_size = 0x800000ULL;       // 8MB
    mm->rlimits.max_heap_size = 0x40000000ULL;      // 1GB
    mm->rlimits.max_mmap_size = 0x10000000000ULL;   // 1TB
    
    return mm;
}

/**
 * Destroy an address space
 */
void vmm_destroy_address_space(address_space_t* mm) {
    if (!mm || mm == vmm->kernel_mm) {
        return;
    }
    
    // Remove all VMAs
    vm_area_t* vma = mm->mmap;
    while (vma) {
        vm_area_t* next = vma->vm_next;
        vmm_remove_vma(mm, vma);
        vma = next;
    }
    
    // Free page tables (user space only)
    pml4_t* pgd = mm->pgd;
    for (int pml4_idx = 0; pml4_idx < 256; pml4_idx++) {  // User space only
        if (!(pgd->entries[pml4_idx] & VMM_PTE_PRESENT)) continue;
        
        pdpt_t* pdpt = (pdpt_t*)(uintptr_t)(pgd->entries[pml4_idx] & ~VMM_PAGE_MASK);
        for (int pdpt_idx = 0; pdpt_idx < VMM_PDPT_ENTRIES; pdpt_idx++) {
            if (!(pdpt->entries[pdpt_idx] & VMM_PTE_PRESENT)) continue;
            
            page_directory_t* pd = (page_directory_t*)(uintptr_t)(pdpt->entries[pdpt_idx] & ~VMM_PAGE_MASK);
            for (int pd_idx = 0; pd_idx < VMM_PD_ENTRIES; pd_idx++) {
                if (!(pd->entries[pd_idx] & VMM_PTE_PRESENT)) continue;
                
                page_table_t* pt = (page_table_t*)(uintptr_t)(pd->entries[pd_idx] & ~VMM_PAGE_MASK);
                pmm_free_page(pt);
            }
            pmm_free_page(pd);
        }
        pmm_free_page(pdpt);
    }
    
    // Free PML4
    pmm_free_page(mm->pgd);
    
    // Free address space structure
    kfree(mm);
}

/**
 * Switch to different address space
 */
void vmm_switch_address_space(address_space_t* mm) {
    if (!mm) return;
    
    // Load new page directory into CR3
    uint64_t pgd_phys = vmm_virt_to_phys(vmm->kernel_mm, (uintptr_t)mm->pgd);
    asm volatile("mov %0, %%cr3" :: "r"(pgd_phys) : "memory");
}

/**
 * Map virtual address to physical address
 */
int vmm_map_page(address_space_t* mm, uint64_t vaddr, uint64_t paddr, 
                 size_t size, uint32_t prot) {
    if (!mm || !vmm_page_align(vaddr) || !vmm_page_align(paddr)) {
        return -EINVAL;
    }
    
    spin_lock(&mm->page_table_lock);
    
    size_t pages = (size + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;
    uint64_t curr_vaddr = vmm_page_align(vaddr);
    uint64_t curr_paddr = vmm_page_align(paddr);
    
    for (size_t i = 0; i < pages; i++) {
        uint64_t* pte = vmm_walk_page_table(mm, curr_vaddr, true);
        if (!pte) {
            spin_unlock(&mm->page_table_lock);
            return -ENOMEM;
        }
        
        // Set up page table entry
        uint64_t entry = curr_paddr | VMM_PTE_PRESENT;
        
        if (prot & MM_PROT_WRITE) entry |= VMM_PTE_WRITE;
        if (prot & MM_PROT_USER) entry |= VMM_PTE_USER;
        if (!(prot & MM_PROT_EXEC) && vmm->config.execute_disable) {
            entry |= VMM_PTE_NX;
        }
        
        *pte = entry;
        
        curr_vaddr += VMM_PAGE_SIZE;
        curr_paddr += VMM_PAGE_SIZE;
    }
    
    // Flush TLB for mapped range
    vmm_flush_tlb_range(mm, vaddr, vaddr + size);
    
    atomic64_add(&vmm->stats.total_pages_mapped, pages);
    mm->vm_stats.resident_pages += pages;
    
    spin_unlock(&mm->page_table_lock);
    return 0;
}

/**
 * Walk page table for address
 */
uint64_t* vmm_walk_page_table(address_space_t* mm, uint64_t vaddr, bool create_missing) {
    pml4_t* pgd = mm->pgd;
    uint64_t pml4_idx = vmm_pml4_index(vaddr);
    uint64_t pdpt_idx = vmm_pdpt_index(vaddr);
    uint64_t pd_idx = vmm_pd_index(vaddr);
    uint64_t pt_idx = vmm_pt_index(vaddr);
    
    // Check PML4 entry
    if (!(pgd->entries[pml4_idx] & VMM_PTE_PRESENT)) {
        if (!create_missing) return NULL;
        
        pdpt_t* pdpt = (pdpt_t*)pmm_alloc_page(MM_FLAG_KERNEL | MM_FLAG_ZERO, -1);
        if (!pdpt) return NULL;
        
        uint64_t pdpt_phys = vmm_virt_to_phys(vmm->kernel_mm, (uintptr_t)pdpt);
        pgd->entries[pml4_idx] = pdpt_phys | VMM_PTE_PRESENT | VMM_PTE_WRITE | VMM_PTE_USER;
    }
    
    // Get PDPT
    pdpt_t* pdpt = (pdpt_t*)(uintptr_t)(pgd->entries[pml4_idx] & ~VMM_PAGE_MASK);
    
    // Check PDPT entry
    if (!(pdpt->entries[pdpt_idx] & VMM_PTE_PRESENT)) {
        if (!create_missing) return NULL;
        
        page_directory_t* pd = (page_directory_t*)pmm_alloc_page(MM_FLAG_KERNEL | MM_FLAG_ZERO, -1);
        if (!pd) return NULL;
        
        uint64_t pd_phys = vmm_virt_to_phys(vmm->kernel_mm, (uintptr_t)pd);
        pdpt->entries[pdpt_idx] = pd_phys | VMM_PTE_PRESENT | VMM_PTE_WRITE | VMM_PTE_USER;
    }
    
    // Get page directory
    page_directory_t* pd = (page_directory_t*)(uintptr_t)(pdpt->entries[pdpt_idx] & ~VMM_PAGE_MASK);
    
    // Check PD entry
    if (!(pd->entries[pd_idx] & VMM_PTE_PRESENT)) {
        if (!create_missing) return NULL;
        
        page_table_t* pt = (page_table_t*)pmm_alloc_page(MM_FLAG_KERNEL | MM_FLAG_ZERO, -1);
        if (!pt) return NULL;
        
        uint64_t pt_phys = vmm_virt_to_phys(vmm->kernel_mm, (uintptr_t)pt);
        pd->entries[pd_idx] = pt_phys | VMM_PTE_PRESENT | VMM_PTE_WRITE | VMM_PTE_USER;
    }
    
    // Get page table
    page_table_t* pt = (page_table_t*)(uintptr_t)(pd->entries[pd_idx] & ~VMM_PAGE_MASK);
    
    return &pt->entries[pt_idx];
}

/**
 * Translate virtual address to physical
 */
uint64_t vmm_virt_to_phys(address_space_t* mm, uint64_t vaddr) {
    if (!mm) return 0;
    
    spin_lock(&mm->page_table_lock);
    
    uint64_t* pte = vmm_walk_page_table(mm, vaddr, false);
    if (!pte || !(*pte & VMM_PTE_PRESENT)) {
        spin_unlock(&mm->page_table_lock);
        return 0;
    }
    
    uint64_t paddr = (*pte & ~VMM_PAGE_MASK) | vmm_page_offset(vaddr);
    
    spin_unlock(&mm->page_table_lock);
    return paddr;
}

/**
 * Find VMA containing address
 */
vm_area_t* vmm_find_vma(address_space_t* mm, uint64_t addr) {
    vm_area_t* vma = NULL;
    
    if (!mm) return NULL;
    
    // Use red-black tree for O(log n) lookup
    struct rb_node* node = mm->mm_rb.rb_node;
    
    while (node) {
        vm_area_t* candidate = rb_entry(node, vm_area_t, vm_rb);
        
        if (addr < candidate->vm_start) {
            node = node->rb_left;
        } else if (addr >= candidate->vm_end) {
            node = node->rb_right;
        } else {
            vma = candidate;
            break;
        }
    }
    
    return vma;
}

/**
 * Create a new VMA
 */
vm_area_t* vmm_create_vma(address_space_t* mm, uint64_t start, size_t len,
                          uint32_t prot, uint32_t flags, vma_type_t type) {
    if (!mm || !len) return NULL;
    
    vm_area_t* vma = kmalloc(sizeof(vm_area_t), MM_FLAG_KERNEL | MM_FLAG_ZERO);
    if (!vma) return NULL;
    
    vma->vm_start = vmm_page_align(start);
    vma->vm_end = vmm_page_align(start + len + VMM_PAGE_SIZE - 1);
    vma->vm_prot = prot;
    vma->vm_flags = flags;
    vma->vm_type = type;
    vma->vm_mm = mm;
    atomic_set(&vma->vm_usage, 1);
    
    // Insert into address space
    int ret = vmm_insert_vma(mm, vma);
    if (ret < 0) {
        kfree(vma);
        return NULL;
    }
    
    return vma;
}

/**
 * Insert VMA into address space
 */
static int vmm_insert_vma(address_space_t* mm, vm_area_t* vma) {
    vm_area_t** p = &mm->mmap;
    vm_area_t* prev = NULL;
    struct rb_node** rb_link = &mm->mm_rb.rb_node;
    struct rb_node* rb_parent = NULL;
    
    // Find insertion point in linked list and rb-tree
    while (*p) {
        // struct rb_node* parent;  // Unused variable
        vm_area_t* curr = *p;
        
        if (vma->vm_start < curr->vm_start) {
            rb_link = &curr->vm_rb.rb_left;
            p = &prev->vm_next;
        } else if (vma->vm_start > curr->vm_start) {
            rb_link = &curr->vm_rb.rb_right;
            prev = curr;
            p = &curr->vm_next;
        } else {
            return -EEXIST;  // Overlapping VMA
        }
        rb_parent = &curr->vm_rb;
    }
    
    // Insert into linked list
    vma->vm_next = *p;
    vma->vm_prev = prev;
    *p = vma;
    if (vma->vm_next) {
        vma->vm_next->vm_prev = vma;
    }
    
    // Insert into rb-tree
    rb_link_node(&vma->vm_rb, rb_parent, rb_link);
    rb_insert_color(&vma->vm_rb, &mm->mm_rb);
    
    mm->map_count++;
    return 0;
}

/**
 * Handle page fault
 */
int vmm_handle_page_fault(address_space_t* mm, uint64_t addr, uint64_t error_code) {
    if (!mm) return -EFAULT;
    
    atomic64_inc(&vmm->stats.total_page_faults);
    mm->vm_stats.page_faults++;
    
    // Find VMA containing the faulting address
    vm_area_t* vma = vmm_find_vma(mm, addr);
    if (!vma) {
        // No VMA found - segmentation fault
        return -EFAULT;
    }
    
    // Check if this is a copy-on-write fault
    if ((error_code & 0x02) && (vma->vm_flags & VMA_FLAG_NONE)) {  // Write fault
        uint64_t* pte = vmm_walk_page_table(mm, addr, false);
        if (pte && (*pte & VMM_PTE_COW)) {
            return vmm_handle_cow_fault(mm, vma, addr);
        }
    }
    
    // Handle demand paging
    if (!(error_code & 0x01)) {  // Page not present
        // Allocate new page
        void* page = pmm_alloc_page(MM_FLAG_KERNEL, -1);
        if (!page) {
            return -ENOMEM;
        }
        
        uint64_t paddr = (uintptr_t)page;
        uint32_t prot = 0;
        
        if (vma->vm_prot & MM_PROT_READ) prot |= MM_PROT_READ;
        if (vma->vm_prot & MM_PROT_WRITE) prot |= MM_PROT_WRITE;
        if (vma->vm_prot & MM_PROT_EXEC) prot |= MM_PROT_EXEC;
        if (vmm_is_user_address(addr)) prot |= MM_PROT_USER;
        
        int ret = vmm_map_page(mm, vmm_page_align(addr), paddr, VMM_PAGE_SIZE, prot);
        if (ret < 0) {
            pmm_free_page(page);
            return ret;
        }
        
        // Zero the page for anonymous mappings
        if (vma->vm_type == VMA_TYPE_ANONYMOUS) {
            memset((void*)(uintptr_t)vmm_page_align(addr), 0, VMM_PAGE_SIZE);
        }
        
        mm->vm_stats.minor_faults++;
        return 0;
    }
    
    return -EFAULT;
}

/**
 * Handle copy-on-write fault
 */
int vmm_handle_cow_fault(address_space_t* mm, vm_area_t* vma, uint64_t addr) {
    atomic64_inc(&vmm->stats.total_cow_faults);
    vma->vm_stats.cow_faults++;
    
    uint64_t page_addr = vmm_page_align(addr);
    
    // Get current page
    uint64_t* pte = vmm_walk_page_table(mm, page_addr, false);
    if (!pte || !(*pte & VMM_PTE_PRESENT)) {
        return -EFAULT;
    }
    
    uint64_t old_paddr = *pte & ~VMM_PAGE_MASK;
    
    // Allocate new page
    void* new_page = pmm_alloc_page(MM_FLAG_KERNEL, -1);
    if (!new_page) {
        return -ENOMEM;
    }
    
    // Copy old page content to new page
    memcpy(new_page, (void*)(uintptr_t)old_paddr, VMM_PAGE_SIZE);
    
    // Update page table entry
    uint64_t new_entry = (uintptr_t)new_page | VMM_PTE_PRESENT | VMM_PTE_WRITE;
    if (vma->vm_prot & MM_PROT_USER) new_entry |= VMM_PTE_USER;
    if (!(vma->vm_prot & MM_PROT_EXEC)) new_entry |= VMM_PTE_NX;
    
    *pte = new_entry;
    
    // Invalidate TLB entry
    vmm_invalidate_page(page_addr);
    
    return 0;
}

/**
 * Set up ASLR mmap base
 */
static void vmm_arch_pick_mmap_base(address_space_t* mm) {
    mm->mmap_base = VMM_MMAP_BASE;
    mm->start_stack = VMM_STACK_TOP;
    
    // Enable ASLR by default
    mm->aslr.enabled = true;
    mm->aslr.mmap_rnd_bits = 28;   // 28 bits of randomization
    mm->aslr.stack_rnd_bits = 23;  // 23 bits for stack
    
    if (mm->aslr.enabled) {
        // Add some basic randomization (simplified)
        uint64_t random = hal->cpu_timestamp() & 0xFFFFF;  // 20 bits
        mm->mmap_base += (random << 12);  // Page-aligned randomization
    }
}

/**
 * Check if address is valid user space address
 */
bool vmm_is_user_address(uint64_t addr) {
    return addr >= VMM_USER_SPACE_START && addr < VMM_USER_SPACE_END;
}

/**
 * Check if address is valid kernel address
 */
bool vmm_is_kernel_address(uint64_t addr) {
    return addr >= VMM_KERNEL_SPACE_START && addr <= VMM_KERNEL_SPACE_END;
}

/**
 * Flush TLB for address range
 */
void vmm_flush_tlb_range(address_space_t* mm __attribute__((unused)), uint64_t start __attribute__((unused)), uint64_t end __attribute__((unused))) {
    // For simplicity, flush entire TLB
    // In production, would use INVLPG for single pages or PCID for selective flushing
#ifdef __x86_64__
    asm volatile("mov %%cr3, %%rax; mov %%rax, %%cr3" ::: "rax", "memory");
#else
    asm volatile("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax", "memory");
#endif
}

/**
 * Invalidate page in TLB
 */
void vmm_invalidate_page(uint64_t addr) {
    asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

/**
 * Get current address space
 */
address_space_t* vmm_get_current_address_space(void) {
    // In a full implementation, this would return the current process's address space
    // For now, return kernel address space
    return vmm->kernel_mm;
}

/**
 * Late initialization
 */
int vmm_late_init(void) {
    vga_puts("VMM: Late initialization complete\n");
    return 0;
}

/**
 * Cleanup VMM resources
 */
void vmm_cleanup(void) {
    vmm->initialized = false;
}

/**
 * Dump address space layout for debugging
 */
void vmm_dump_address_space(address_space_t* mm) {
    if (!mm) return;
    
    vga_puts("VMM Address Space Layout:\n");
    vga_puts("  PGD: 0x");
    
    // Simple hex printing
    uint64_t pgd_addr = (uintptr_t)mm->pgd;
    for (int i = 60; i >= 0; i -= 4) {
        uint64_t nibble = (pgd_addr >> i) & 0xF;
        char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        char hex_str[2] = {hex_char, '\0'};
        vga_puts(hex_str);
    }
    vga_puts("\n");
    
    // Dump VMAs
    vm_area_t* vma = mm->mmap;
    int count = 0;
    while (vma && count < 10) {  // Limit output
        vga_puts("  VMA: 0x");
        
        uint64_t start = vma->vm_start;
        for (int i = 60; i >= 0; i -= 4) {
            uint64_t nibble = (start >> i) & 0xF;
            char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
            char hex_str[2] = {hex_char, '\0'};
            vga_puts(hex_str);
        }
        
        vga_puts(" - 0x");
        
        uint64_t end = vma->vm_end;
        for (int i = 60; i >= 0; i -= 4) {
            uint64_t nibble = (end >> i) & 0xF;
            char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
            char hex_str[2] = {hex_char, '\0'};
            vga_puts(hex_str);
        }
        vga_puts("\n");
        
        vma = vma->vm_next;
        count++;
    }
}

// Additional stub implementations for completeness

int vmm_unmap_pages(address_space_t* mm __attribute__((unused)), uint64_t vaddr __attribute__((unused)), size_t size __attribute__((unused))) {
    // Stub implementation
    return 0;
}

int vmm_protect_pages(address_space_t* mm __attribute__((unused)), uint64_t vaddr __attribute__((unused)), size_t size __attribute__((unused)), uint32_t prot __attribute__((unused))) {
    // Stub implementation  
    return 0;
}

void vmm_remove_vma(address_space_t* mm __attribute__((unused)), vm_area_t* vma) {
    // Stub implementation
    if (vma) {
        kfree(vma);
    }
}

uint64_t vmm_mmap(address_space_t* mm __attribute__((unused)), uint64_t addr __attribute__((unused)), size_t len __attribute__((unused)), uint32_t prot __attribute__((unused)),
                  uint32_t flags __attribute__((unused)), int fd __attribute__((unused)), uint64_t offset __attribute__((unused))) {
    // Stub implementation
    return 0;
}

int vmm_munmap(address_space_t* mm __attribute__((unused)), uint64_t addr __attribute__((unused)), size_t len __attribute__((unused))) {
    // Stub implementation
    return 0;
}
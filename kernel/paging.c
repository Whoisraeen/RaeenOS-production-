#include "paging.h"
#include "pmm.h"
#include "string.h"

// Kernel's PML4
static pml4_t* kernel_pml4 = NULL;

void vmm_init() {
    // Create a new address space for the kernel
    kernel_pml4 = vmm_create_address_space();

    // Identity map the first 4GB of physical memory
    for (uint64_t i = 0; i < 0x100000000; i += PAGE_SIZE) {
        vmm_map_page(kernel_pml4, i, i, PTE_WRITE | PTE_PRESENT);
    }

    // Switch to the new address space
    vmm_switch_address_space(kernel_pml4);
}

void vmm_map_page(pml4_t* pml4, uintptr_t vaddr, uintptr_t paddr, uint64_t flags) {
    uint64_t pml4_index = PML4_INDEX(vaddr);
    uint64_t pdpt_index = PDPT_INDEX(vaddr);
    uint64_t pd_index = PD_INDEX(vaddr);
    uint64_t pt_index = PT_INDEX(vaddr);

    pdpt_t* pdpt;
    if (pml4->entries[pml4_index] & PTE_PRESENT) {
        pdpt = (pdpt_t*)(pml4->entries[pml4_index] & ~0xFFF);
    } else {
        pdpt = pmm_alloc_frame();
        memset(pdpt, 0, PAGE_SIZE);
        pml4->entries[pml4_index] = (uint64_t)pdpt | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    pd_t* pd;
    if (pdpt->entries[pdpt_index] & PTE_PRESENT) {
        pd = (pd_t*)(pdpt->entries[pdpt_index] & ~0xFFF);
    } else {
        pd = pmm_alloc_frame();
        memset(pd, 0, PAGE_SIZE);
        pdpt->entries[pdpt_index] = (uint64_t)pd | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    pt_t* pt;
    if (pd->entries[pd_index] & PTE_PRESENT) {
        pt = (pt_t*)(pd->entries[pd_index] & ~0xFFF);
    } else {
        pt = pmm_alloc_frame();
        memset(pt, 0, PAGE_SIZE);
        pd->entries[pd_index] = (uint64_t)pt | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    pt->entries[pt_index] = paddr | flags;
}

void vmm_unmap_page(pml4_t* pml4, uintptr_t vaddr) {
    uint64_t pml4_index = PML4_INDEX(vaddr);
    uint64_t pdpt_index = PDPT_INDEX(vaddr);
    uint64_t pd_index = PD_INDEX(vaddr);
    uint64_t pt_index = PT_INDEX(vaddr);

    if (!(pml4->entries[pml4_index] & PTE_PRESENT)) return;
    pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_index] & ~0xFFF);

    if (!(pdpt->entries[pdpt_index] & PTE_PRESENT)) return;
    pd_t* pd = (pd_t*)(pdpt->entries[pdpt_index] & ~0xFFF);

    if (!(pd->entries[pd_index] & PTE_PRESENT)) return;
    pt_t* pt = (pt_t*)(pd->entries[pd_index] & ~0xFFF);

    pt->entries[pt_index] = 0; // Clear the entry

    // Invalidate TLB for the unmapped page
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

uintptr_t vmm_get_physical_address(pml4_t* pml4, uintptr_t vaddr) {
    uint64_t pml4_index = PML4_INDEX(vaddr);
    uint64_t pdpt_index = PDPT_INDEX(vaddr);
    uint64_t pd_index = PD_INDEX(vaddr);
    uint64_t pt_index = PT_INDEX(vaddr);

    if (!(pml4->entries[pml4_index] & PTE_PRESENT)) return 0;
    pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_index] & ~0xFFF);

    if (!(pdpt->entries[pdpt_index] & PTE_PRESENT)) return 0;
    pd_t* pd = (pd_t*)(pdpt->entries[pdpt_index] & ~0xFFF);

    if (!(pd->entries[pd_index] & PTE_PRESENT)) return 0;
    pt_t* pt = (pt_t*)(pd->entries[pd_index] & ~0xFFF);

    uintptr_t page_frame = pt->entries[pt_index] & ~0xFFF; // Mask out flags
    return page_frame + (vaddr % PAGE_SIZE);
}

pml4_t* vmm_create_address_space() {
    pml4_t* pml4 = pmm_alloc_frame();
    memset(pml4, 0, PAGE_SIZE);
    return pml4;
}

void vmm_switch_address_space(pml4_t* pml4) {
    asm volatile("mov %0, %%cr3" : : "r"(pml4));
}

void vmm_set_page_executable(pml4_t* pml4, uintptr_t vaddr) {
    uint64_t pml4_index = PML4_INDEX(vaddr);
    uint64_t pdpt_index = PDPT_INDEX(vaddr);
    uint64_t pd_index = PD_INDEX(vaddr);
    uint64_t pt_index = PT_INDEX(vaddr);

    if (!(pml4->entries[pml4_index] & PTE_PRESENT)) return;
    pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_index] & ~0xFFF);

    if (!(pdpt->entries[pdpt_index] & PTE_PRESENT)) return;
    pd_t* pd = (pd_t*)(pdpt->entries[pdpt_index] & ~0xFFF);

    if (!(pd->entries[pd_index] & PTE_PRESENT)) return;
    pt_t* pt = (pt_t*)(pd->entries[pd_index] & ~0xFFF);

    pt->entries[pt_index] &= ~PTE_NO_EXECUTE; // Clear NX bit
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory"); // Invalidate TLB
}

void vmm_set_page_non_executable(pml4_t* pml4, uintptr_t vaddr) {
    uint64_t pml4_index = PML4_INDEX(vaddr);
    uint64_t pdpt_index = PDPT_INDEX(vaddr);
    uint64_t pd_index = PD_INDEX(vaddr);
    uint64_t pt_index = PT_INDEX(vaddr);

    if (!(pml4->entries[pml4_index] & PTE_PRESENT)) return;
    pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_index] & ~0xFFF);

    if (!(pdpt->entries[pdpt_index] & PTE_PRESENT)) return;
    pd_t* pd = (pd_t*)(pdpt->entries[pdpt_index] & ~0xFFF);

    if (!(pd->entries[pd_index] & PTE_PRESENT)) return;
    pt_t* pt = (pt_t*)(pd->entries[pd_index] & ~0xFFF);

    pt->entries[pt_index] |= PTE_NO_EXECUTE; // Set NX bit
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory"); // Invalidate TLB
}
#include <stdbool.h>
#include "swap.h"
#include "vga.h"
#include "pmm.h"
#include "paging.h"
#include "../drivers/ata/ata.h"

// For now, a very simplistic swap space on the first ATA drive
#define SWAP_START_LBA 1024 // Start after bootloader/kernel
#define SWAP_SIZE_SECTORS 1024 // 512KB swap space (1024 sectors * 512 bytes/sector)
#define ATA_MASTER 0 // Master drive

void swap_init(void) {
    vga_puts("Swap initialized (placeholder).\n");
}

bool swap_out_page(uint32_t physical_address) {
    // Find a free sector in swap space
    // For now, just use a dummy sector
    uint32_t swap_sector = SWAP_START_LBA; 

    // Read the page into a buffer
    uint8_t* buffer = (uint8_t*)physical_address;

    // Write the buffer to the swap sector
    if (ata_write_sectors(ATA_MASTER, swap_sector, 1, buffer) != 0) {
        vga_puts("Swap out failed!\n");
        return false;
    }

    // Mark the physical page as free (or reserved for swap)
    pmm_free_frame((void*)physical_address);

    vga_puts("Page swapped out.\n");
    return true;
}

bool swap_in_page(uint32_t physical_address) {
    // Find the corresponding sector in swap space
    // For now, just use a dummy sector
    uint32_t swap_sector = SWAP_START_LBA;

    // Allocate a physical frame for the page
    void* new_frame = pmm_alloc_frame();
    if (!new_frame) {
        vga_puts("Swap in failed: Out of memory!\n");
        return false;
    }

    // Read the swap sector into the new frame
    if (ata_read_sectors(ATA_MASTER, swap_sector, 1, (uint8_t*)new_frame) != 0) {
        vga_puts("Swap in failed!\n");
        pmm_free_frame(new_frame);
        return false;
    }

    // Update page table entry to point to new physical address
    // This part would typically be handled by the page fault handler
    // For now, we just return true.

    vga_puts("Page swapped in.\n");
    return true;
}

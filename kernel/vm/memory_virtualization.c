#include "memory_virtualization.h"
#include "../vga.h"

void mem_virt_init(void) {
    debug_print("Memory Virtualization initialized (placeholder).\n");
}

int mem_virt_map_guest_memory(uint64_t guest_paddr, uint64_t host_paddr, size_t size) {
    debug_print("Mem Virt: Mapping Guest PAddr ");
    vga_put_hex(guest_paddr);
    debug_print(" to Host PAddr ");
    vga_put_hex(host_paddr);
    debug_print(" (simulated).\n");
    return 0; // Success
}

int mem_virt_unmap_guest_memory(uint64_t guest_paddr, size_t size) {
    debug_print("Mem Virt: Unmapping Guest PAddr ");
    vga_put_hex(guest_paddr);
    debug_print(" (simulated).\n");
    return 0; // Success
}


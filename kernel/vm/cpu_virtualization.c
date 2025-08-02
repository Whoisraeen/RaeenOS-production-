#include "cpu_virtualization.h"
#include "../vga.h"
#include "../memory.h"

void cpu_virt_init(void) {
    debug_print("CPU Virtualization initialized (placeholder).\n");
}

void* cpu_virt_create_vcpu(uint32_t vcpu_id) {
    debug_print("CPU Virt: Creating vCPU ");
    vga_put_dec(vcpu_id);
    debug_print(" (simulated).\n");
    return kmalloc(1); // Dummy handle
}

int cpu_virt_run_vcpu(void* vcpu_handle) {
    debug_print("CPU Virt: Running vCPU (simulated).\n");
    return 0; // Success
}

void cpu_virt_handle_vm_exit(void* vcpu_handle) {
    debug_print("CPU Virt: Handling VM exit (simulated).\n");
}


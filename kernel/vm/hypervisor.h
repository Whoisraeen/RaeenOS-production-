#ifndef HYPERVISOR_H
#define HYPERVISOR_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct vm_context vm_context_t;

// Initialize the hypervisor subsystem
void hypervisor_init(void);

// Create a new virtual machine (placeholder)
vm_context_t* hypervisor_create_vm(uint32_t memory_size_mb, uint32_t num_vcpus);

// Start a virtual machine (placeholder)
int hypervisor_start_vm(vm_context_t* vm);

// Stop a virtual machine (placeholder)
int hypervisor_stop_vm(vm_context_t* vm);

// Destroy a virtual machine (placeholder)
void hypervisor_destroy_vm(vm_context_t* vm);

// Execute a virtual CPU (placeholder)
int hypervisor_vcpu_run(vm_context_t* vm, uint32_t vcpu_id);

#endif // HYPERVISOR_H

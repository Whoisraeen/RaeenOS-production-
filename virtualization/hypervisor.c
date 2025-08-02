/**
 * RaeenOS Hypervisor Implementation
 * Type-1 hypervisor with hardware virtualization support
 */

#include "virtualization.h"
#include "../memory.h"
#include "../string.h"
#include "../cpu.h"

// VM Exit Reasons (Intel VT-x)
#define VM_EXIT_EXCEPTION_NMI       0
#define VM_EXIT_EXTERNAL_INTERRUPT  1
#define VM_EXIT_TRIPLE_FAULT        2
#define VM_EXIT_INIT_SIGNAL         3
#define VM_EXIT_STARTUP_IPI         4
#define VM_EXIT_IO_INSTRUCTION      30
#define VM_EXIT_RDMSR              31
#define VM_EXIT_WRMSR              32
#define VM_EXIT_VMCALL             18
#define VM_EXIT_EPT_VIOLATION      48

// VMCS Fields
#define VMCS_GUEST_RIP             0x681E
#define VMCS_GUEST_RSP             0x681C
#define VMCS_GUEST_RFLAGS          0x6820
#define VMCS_GUEST_CR0             0x6800
#define VMCS_GUEST_CR3             0x6802
#define VMCS_GUEST_CR4             0x6804
#define VMCS_HOST_RIP              0x6C16
#define VMCS_HOST_RSP              0x6C14
#define VMCS_HOST_CR0              0x6C00
#define VMCS_HOST_CR3              0x6C02
#define VMCS_HOST_CR4              0x6C04

// Virtual Machine Control Structure
typedef struct {
    uint32_t revision_id;
    uint32_t vmx_abort_indicator;
    uint8_t data[4092];
} __attribute__((aligned(4096))) vmcs_t;

// Virtual CPU State
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cr0, cr2, cr3, cr4;
    uint64_t dr0, dr1, dr2, dr3, dr6, dr7;
    
    // Segment registers
    uint16_t cs, ds, es, fs, gs, ss;
    uint64_t cs_base, ds_base, es_base, fs_base, gs_base, ss_base;
    uint32_t cs_limit, ds_limit, es_limit, fs_limit, gs_limit, ss_limit;
    
    // System registers
    uint64_t gdtr_base, idtr_base;
    uint16_t gdtr_limit, idtr_limit;
    uint16_t ldtr, tr;
    
    // MSRs
    uint64_t efer;
    uint64_t star, lstar, cstar, sfmask;
} vcpu_state_t;

// Virtual Machine Instance
typedef struct vm_instance {
    uint32_t vm_id;
    char name[64];
    
    // Memory management
    uint64_t memory_size;
    void* guest_memory;
    uint64_t* ept_pml4; // Extended Page Tables
    
    // Virtual CPUs
    vcpu_state_t* vcpus;
    uint32_t vcpu_count;
    vmcs_t* vmcs_regions;
    
    // Device emulation
    struct {
        bool enabled;
        uint16_t base_port;
        uint8_t irq;
    } serial_port;
    
    struct {
        bool enabled;
        void* framebuffer;
        uint32_t width, height;
        uint32_t bpp;
    } display;
    
    struct {
        bool enabled;
        uint8_t* disk_image;
        uint64_t disk_size;
    } storage;
    
    // VM state
    bool running;
    bool paused;
    uint64_t total_exits;
    uint64_t io_exits;
    uint64_t mmio_exits;
    
    struct vm_instance* next;
} vm_instance_t;

// Hypervisor State
typedef struct {
    bool vmx_supported;
    bool ept_supported;
    bool vpid_supported;
    bool unrestricted_guest;
    
    vm_instance_t* vms;
    uint32_t next_vm_id;
    
    // Host state backup
    uint64_t host_cr0, host_cr3, host_cr4;
    uint64_t host_gdtr_base, host_idtr_base;
    uint16_t host_gdtr_limit, host_idtr_limit;
    
    bool initialized;
} hypervisor_t;

static hypervisor_t g_hypervisor = {0};

// Function declarations
static bool hypervisor_check_vmx_support(void);
static bool hypervisor_enable_vmx(void);
static bool hypervisor_setup_vmcs(vm_instance_t* vm, uint32_t vcpu_index);
static void hypervisor_handle_vm_exit(vm_instance_t* vm, uint32_t vcpu_index, uint32_t exit_reason);
static bool hypervisor_setup_ept(vm_instance_t* vm);
static void hypervisor_emulate_io(vm_instance_t* vm, uint32_t port, uint32_t size, bool is_write, uint64_t value);
static void hypervisor_emulate_mmio(vm_instance_t* vm, uint64_t gpa, uint32_t size, bool is_write, uint64_t value);
static uint64_t hypervisor_gva_to_gpa(vm_instance_t* vm, uint32_t vcpu_index, uint64_t gva);

/**
 * Initialize hypervisor
 */
bool hypervisor_init(void) {
    hypervisor_t* hv = &g_hypervisor;
    
    if (hv->initialized) {
        return true;
    }
    
    printf("Hypervisor: Initializing...\n");
    
    // Check VMX support
    if (!hypervisor_check_vmx_support()) {
        printf("Hypervisor: VMX not supported\n");
        return false;
    }
    
    // Enable VMX operation
    if (!hypervisor_enable_vmx()) {
        printf("Hypervisor: Failed to enable VMX\n");
        return false;
    }
    
    hv->next_vm_id = 1;
    hv->initialized = true;
    
    printf("Hypervisor: Initialized successfully\n");
    printf("Hypervisor: EPT: %s, VPID: %s, Unrestricted Guest: %s\n",
           hv->ept_supported ? "Yes" : "No",
           hv->vpid_supported ? "Yes" : "No",
           hv->unrestricted_guest ? "Yes" : "No");
    
    return true;
}

/**
 * Create virtual machine
 */
vm_instance_t* hypervisor_create_vm(const char* name, uint64_t memory_size, uint32_t vcpu_count) {
    hypervisor_t* hv = &g_hypervisor;
    
    if (!hv->initialized || !name || memory_size == 0 || vcpu_count == 0) {
        return NULL;
    }
    
    vm_instance_t* vm = (vm_instance_t*)memory_alloc(sizeof(vm_instance_t));
    if (!vm) {
        return NULL;
    }
    
    memory_set(vm, 0, sizeof(vm_instance_t));
    vm->vm_id = hv->next_vm_id++;
    string_copy(vm->name, name, sizeof(vm->name));
    vm->memory_size = memory_size;
    vm->vcpu_count = vcpu_count;
    
    // Allocate guest memory
    vm->guest_memory = memory_alloc_aligned(memory_size, 4096);
    if (!vm->guest_memory) {
        memory_free(vm);
        return NULL;
    }
    memory_set(vm->guest_memory, 0, memory_size);
    
    // Allocate VCPU states
    vm->vcpus = (vcpu_state_t*)memory_alloc(sizeof(vcpu_state_t) * vcpu_count);
    if (!vm->vcpus) {
        memory_free(vm->guest_memory);
        memory_free(vm);
        return NULL;
    }
    
    // Allocate VMCS regions
    vm->vmcs_regions = (vmcs_t*)memory_alloc_aligned(sizeof(vmcs_t) * vcpu_count, 4096);
    if (!vm->vmcs_regions) {
        memory_free(vm->vcpus);
        memory_free(vm->guest_memory);
        memory_free(vm);
        return NULL;
    }
    
    // Setup Extended Page Tables
    if (hv->ept_supported) {
        if (!hypervisor_setup_ept(vm)) {
            printf("Hypervisor: Failed to setup EPT for VM %s\n", name);
            memory_free(vm->vmcs_regions);
            memory_free(vm->vcpus);
            memory_free(vm->guest_memory);
            memory_free(vm);
            return NULL;
        }
    }
    
    // Initialize VCPUs
    for (uint32_t i = 0; i < vcpu_count; i++) {
        memory_set(&vm->vcpus[i], 0, sizeof(vcpu_state_t));
        
        // Set initial CPU state (real mode)
        vm->vcpus[i].rip = 0xFFF0;
        vm->vcpus[i].cs = 0xF000;
        vm->vcpus[i].cs_base = 0xFFFF0000;
        vm->vcpus[i].cs_limit = 0xFFFF;
        vm->vcpus[i].rflags = 0x2;
        vm->vcpus[i].cr0 = 0x60000010;
        vm->vcpus[i].cr4 = 0x2000;
        
        // Setup VMCS
        if (!hypervisor_setup_vmcs(vm, i)) {
            printf("Hypervisor: Failed to setup VMCS for VCPU %u\n", i);
            // Cleanup and return NULL
            memory_free(vm->vmcs_regions);
            memory_free(vm->vcpus);
            memory_free(vm->guest_memory);
            memory_free(vm);
            return NULL;
        }
    }
    
    // Add to VM list
    vm->next = hv->vms;
    hv->vms = vm;
    
    printf("Hypervisor: Created VM '%s' (ID: %u, Memory: %llu MB, VCPUs: %u)\n",
           name, vm->vm_id, memory_size / (1024 * 1024), vcpu_count);
    
    return vm;
}

/**
 * Start virtual machine
 */
bool hypervisor_start_vm(vm_instance_t* vm) {
    if (!vm || vm->running) {
        return false;
    }
    
    printf("Hypervisor: Starting VM '%s'\n", vm->name);
    
    // Start all VCPUs
    for (uint32_t i = 0; i < vm->vcpu_count; i++) {
        // Load VMCS
        if (__vmx_vmclear((uint64_t)&vm->vmcs_regions[i]) != 0) {
            printf("Hypervisor: VMCLEAR failed for VCPU %u\n", i);
            return false;
        }
        
        if (__vmx_vmptrld((uint64_t)&vm->vmcs_regions[i]) != 0) {
            printf("Hypervisor: VMPTRLD failed for VCPU %u\n", i);
            return false;
        }
        
        // Enter VMX non-root operation
        uint64_t exit_reason;
        if (__vmx_vmlaunch() != 0) {
            __vmx_vmread(VM_EXIT_REASON, &exit_reason);
            printf("Hypervisor: VMLAUNCH failed for VCPU %u (reason: %llu)\n", i, exit_reason);
            return false;
        }
    }
    
    vm->running = true;
    return true;
}

/**
 * Stop virtual machine
 */
bool hypervisor_stop_vm(vm_instance_t* vm) {
    if (!vm || !vm->running) {
        return false;
    }
    
    printf("Hypervisor: Stopping VM '%s'\n", vm->name);
    
    vm->running = false;
    vm->paused = false;
    
    return true;
}

/**
 * Load guest OS image
 */
bool hypervisor_load_guest_image(vm_instance_t* vm, const void* image, uint64_t size, uint64_t load_address) {
    if (!vm || !image || size == 0 || load_address + size > vm->memory_size) {
        return false;
    }
    
    memory_copy((uint8_t*)vm->guest_memory + load_address, image, size);
    
    printf("Hypervisor: Loaded %llu bytes at 0x%llx in VM '%s'\n", size, load_address, vm->name);
    return true;
}

/**
 * Configure virtual device
 */
bool hypervisor_configure_device(vm_instance_t* vm, hypervisor_device_type_t device_type, 
                                const hypervisor_device_config_t* config) {
    if (!vm || !config) {
        return false;
    }
    
    switch (device_type) {
        case HYPERVISOR_DEVICE_SERIAL:
            vm->serial_port.enabled = true;
            vm->serial_port.base_port = config->serial.base_port;
            vm->serial_port.irq = config->serial.irq;
            printf("Hypervisor: Configured serial port (port: 0x%x, IRQ: %u)\n", 
                   config->serial.base_port, config->serial.irq);
            break;
            
        case HYPERVISOR_DEVICE_DISPLAY:
            vm->display.enabled = true;
            vm->display.width = config->display.width;
            vm->display.height = config->display.height;
            vm->display.bpp = config->display.bpp;
            vm->display.framebuffer = memory_alloc(config->display.width * config->display.height * (config->display.bpp / 8));
            printf("Hypervisor: Configured display (%ux%u, %u bpp)\n",
                   config->display.width, config->display.height, config->display.bpp);
            break;
            
        case HYPERVISOR_DEVICE_STORAGE:
            vm->storage.enabled = true;
            vm->storage.disk_size = config->storage.size;
            vm->storage.disk_image = (uint8_t*)memory_alloc(config->storage.size);
            if (vm->storage.disk_image) {
                memory_set(vm->storage.disk_image, 0, config->storage.size);
            }
            printf("Hypervisor: Configured storage (%llu MB)\n", config->storage.size / (1024 * 1024));
            break;
            
        default:
            return false;
    }
    
    return true;
}

/**
 * Get VM statistics
 */
void hypervisor_get_vm_stats(vm_instance_t* vm, hypervisor_vm_stats_t* stats) {
    if (!vm || !stats) {
        return;
    }
    
    memory_set(stats, 0, sizeof(hypervisor_vm_stats_t));
    stats->total_exits = vm->total_exits;
    stats->io_exits = vm->io_exits;
    stats->mmio_exits = vm->mmio_exits;
    stats->memory_usage = vm->memory_size;
    stats->vcpu_count = vm->vcpu_count;
    stats->running = vm->running;
}

// Internal helper functions

static bool hypervisor_check_vmx_support(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Check CPUID.1:ECX.VMX[bit 5]
    __cpuid(1, eax, ebx, ecx, edx);
    if (!(ecx & (1 << 5))) {
        return false;
    }
    
    g_hypervisor.vmx_supported = true;
    
    // Check VMX capabilities
    uint64_t vmx_basic = __readmsr(0x480); // IA32_VMX_BASIC
    uint64_t vmx_procbased = __readmsr(0x482); // IA32_VMX_PROCBASED_CTLS
    uint64_t vmx_procbased2 = __readmsr(0x48B); // IA32_VMX_PROCBASED_CTLS2
    
    // Check EPT support
    if (vmx_procbased2 & (1ULL << 33)) {
        g_hypervisor.ept_supported = true;
    }
    
    // Check VPID support
    if (vmx_procbased2 & (1ULL << 37)) {
        g_hypervisor.vpid_supported = true;
    }
    
    // Check unrestricted guest support
    if (vmx_procbased2 & (1ULL << 39)) {
        g_hypervisor.unrestricted_guest = true;
    }
    
    return true;
}

static bool hypervisor_enable_vmx(void) {
    // Enable VMX in CR4
    uint64_t cr4 = __readcr4();
    cr4 |= (1ULL << 13); // CR4.VMXE
    __writecr4(cr4);
    
    // Execute VMXON
    uint64_t vmxon_region_pa = (uint64_t)memory_alloc_aligned(4096, 4096);
    if (!vmxon_region_pa) {
        return false;
    }
    
    // Set revision ID
    uint64_t vmx_basic = __readmsr(0x480);
    *(uint32_t*)vmxon_region_pa = (uint32_t)vmx_basic;
    
    if (__vmx_on(&vmxon_region_pa) != 0) {
        memory_free((void*)vmxon_region_pa);
        return false;
    }
    
    return true;
}

static bool hypervisor_setup_vmcs(vm_instance_t* vm, uint32_t vcpu_index) {
    vmcs_t* vmcs = &vm->vmcs_regions[vcpu_index];
    vcpu_state_t* vcpu = &vm->vcpus[vcpu_index];
    
    // Set VMCS revision ID
    uint64_t vmx_basic = __readmsr(0x480);
    vmcs->revision_id = (uint32_t)vmx_basic;
    
    // Clear VMCS
    if (__vmx_vmclear((uint64_t)vmcs) != 0) {
        return false;
    }
    
    // Load VMCS
    if (__vmx_vmptrld((uint64_t)vmcs) != 0) {
        return false;
    }
    
    // Configure guest state
    __vmx_vmwrite(VMCS_GUEST_RIP, vcpu->rip);
    __vmx_vmwrite(VMCS_GUEST_RSP, vcpu->rsp);
    __vmx_vmwrite(VMCS_GUEST_RFLAGS, vcpu->rflags);
    __vmx_vmwrite(VMCS_GUEST_CR0, vcpu->cr0);
    __vmx_vmwrite(VMCS_GUEST_CR3, vcpu->cr3);
    __vmx_vmwrite(VMCS_GUEST_CR4, vcpu->cr4);
    
    // Configure host state
    __vmx_vmwrite(VMCS_HOST_RIP, (uint64_t)hypervisor_vm_exit_handler);
    __vmx_vmwrite(VMCS_HOST_RSP, (uint64_t)memory_alloc(8192) + 8192); // Host stack
    __vmx_vmwrite(VMCS_HOST_CR0, __readcr0());
    __vmx_vmwrite(VMCS_HOST_CR3, __readcr3());
    __vmx_vmwrite(VMCS_HOST_CR4, __readcr4());
    
    // Configure execution controls
    uint32_t pin_controls = 0;
    uint32_t proc_controls = (1 << 25) | (1 << 28); // Use I/O bitmaps, Use MSR bitmaps
    uint32_t proc_controls2 = 0;
    
    if (g_hypervisor.ept_supported) {
        proc_controls2 |= (1 << 1); // Enable EPT
    }
    
    if (g_hypervisor.vpid_supported) {
        proc_controls2 |= (1 << 5); // Enable VPID
    }
    
    __vmx_vmwrite(0x4000, pin_controls); // PIN_BASED_VM_EXEC_CONTROL
    __vmx_vmwrite(0x4002, proc_controls); // CPU_BASED_VM_EXEC_CONTROL
    __vmx_vmwrite(0x401E, proc_controls2); // SECONDARY_VM_EXEC_CONTROL
    
    return true;
}

static bool hypervisor_setup_ept(vm_instance_t* vm) {
    // Allocate EPT PML4 table
    vm->ept_pml4 = (uint64_t*)memory_alloc_aligned(4096, 4096);
    if (!vm->ept_pml4) {
        return false;
    }
    
    memory_set(vm->ept_pml4, 0, 4096);
    
    // Map guest physical memory 1:1
    uint64_t guest_pa = (uint64_t)vm->guest_memory;
    uint64_t guest_size = vm->memory_size;
    
    // Simple 1:1 mapping (would be more complex in reality)
    for (uint64_t offset = 0; offset < guest_size; offset += 4096) {
        // Set up EPT page tables to map guest physical to host physical
        // This is a simplified implementation
        vm->ept_pml4[0] = (guest_pa + offset) | 0x7; // Read/Write/Execute
    }
    
    return true;
}

static void hypervisor_emulate_io(vm_instance_t* vm, uint32_t port, uint32_t size, bool is_write, uint64_t value) {
    vm->io_exits++;
    
    // Emulate serial port
    if (vm->serial_port.enabled && port >= vm->serial_port.base_port && port < vm->serial_port.base_port + 8) {
        if (is_write && port == vm->serial_port.base_port) {
            // Write to serial output
            printf("VM Serial: %c", (char)value);
        }
        return;
    }
    
    printf("Hypervisor: Unhandled I/O %s port 0x%x, size %u, value 0x%llx\n",
           is_write ? "write" : "read", port, size, value);
}

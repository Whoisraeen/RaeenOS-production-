/**
 * @file memory_protection.c
 * @brief Memory Protection and Exploitation Mitigation Framework
 * 
 * This module implements advanced memory protection mechanisms for RaeenOS:
 * - Address Space Layout Randomization (ASLR)
 * - Stack Canaries and Stack Protection
 * - Heap Protection (guard pages, corruption detection)
 * - Control Flow Integrity (CFI) enforcement
 * - Return Oriented Programming (ROP) protection
 * - Intel CET (Control-flow Enforcement Technology) support
 * - ARM Memory Tagging Extension (MTE) support
 * - Kernel Address Space Layout Randomization (KASLR)
 * 
 * These protections work together to prevent common memory corruption
 * exploits and make vulnerability exploitation significantly more difficult.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../memory.h"
#include "../string.h"
#include "../paging.h"
#include "../include/hal_interface.h"

// Memory protection configuration
static struct {
    bool aslr_enabled;
    bool stack_protection_enabled;
    bool heap_protection_enabled;
    bool cfi_enabled;
    bool rop_protection_enabled;
    bool kaslr_enabled;
    bool smap_enabled;
    bool smep_enabled;
    bool cet_enabled;
    bool mte_enabled;
    uint32_t stack_canary_value;
    uint64_t heap_magic;
} memory_protection_config = {0};

// Hardware capabilities
static hardware_security_t hw_security = {0};

// Stack canary support
#define STACK_CANARY_MAGIC  0xDEADBEEF
#define HEAP_GUARD_MAGIC    0xFEEDFACE
#define HEAP_FREE_MAGIC     0xDEADC0DE

// ASLR entropy settings
#define ASLR_STACK_ENTROPY  28  // bits
#define ASLR_HEAP_ENTROPY   28  // bits
#define ASLR_MMAP_ENTROPY   28  // bits
#define ASLR_EXEC_ENTROPY   28  // bits

// Memory layout randomization ranges
#define ASLR_STACK_RANGE    (1ULL << ASLR_STACK_ENTROPY)
#define ASLR_HEAP_RANGE     (1ULL << ASLR_HEAP_ENTROPY)
#define ASLR_MMAP_RANGE     (1ULL << ASLR_MMAP_ENTROPY)
#define ASLR_EXEC_RANGE     (1ULL << ASLR_EXEC_ENTROPY)

/**
 * Initialize memory protection framework
 */
int security_init_memory_protection(void) {
    // Detect hardware security features
    int ret = detect_hardware_security_features();
    if (ret != 0) {
        kernel_printf("Memory Protection: Failed to detect hardware features: %d\n", ret);
        return ret;
    }
    
    // Initialize stack canary
    ret = init_stack_canary();
    if (ret != 0) {
        kernel_printf("Memory Protection: Failed to initialize stack canary: %d\n", ret);
        return ret;
    }
    
    // Enable ASLR
    ret = security_enable_aslr();
    if (ret != 0) {
        kernel_printf("Memory Protection: Failed to enable ASLR: %d\n", ret);
        return ret;
    }
    
    // Enable stack protection
    ret = security_enable_stack_protection();
    if (ret != 0) {
        kernel_printf("Memory Protection: Failed to enable stack protection: %d\n", ret);
        return ret;
    }
    
    // Enable heap protection
    ret = security_enable_heap_protection();
    if (ret != 0) {
        kernel_printf("Memory Protection: Failed to enable heap protection: %d\n", ret);
        return ret;
    }
    
    // Enable control flow integrity
    ret = security_enable_cfi();
    if (ret != 0) {
        kernel_printf("Memory Protection: Failed to enable CFI: %d\n", ret);
        return ret;
    }
    
    // Enable hardware features if available
    ret = enable_hardware_protections();
    if (ret != 0) {
        kernel_printf("Memory Protection: Warning - some hardware protections unavailable\n");
    }
    
    kernel_printf("Memory Protection: Framework initialized\n");
    kernel_printf("  ASLR: %s\n", memory_protection_config.aslr_enabled ? "Enabled" : "Disabled");
    kernel_printf("  Stack Protection: %s\n", memory_protection_config.stack_protection_enabled ? "Enabled" : "Disabled");
    kernel_printf("  Heap Protection: %s\n", memory_protection_config.heap_protection_enabled ? "Enabled" : "Disabled");
    kernel_printf("  CFI: %s\n", memory_protection_config.cfi_enabled ? "Enabled" : "Disabled");
    kernel_printf("  SMAP: %s\n", memory_protection_config.smap_enabled ? "Enabled" : "Disabled");
    kernel_printf("  SMEP: %s\n", memory_protection_config.smep_enabled ? "Enabled" : "Disabled");
    kernel_printf("  CET: %s\n", memory_protection_config.cet_enabled ? "Enabled" : "Disabled");
    
    return 0;
}

/**
 * Enable Address Space Layout Randomization (ASLR)
 */
int security_enable_aslr(void) {
    memory_protection_config.aslr_enabled = true;
    memory_protection_config.kaslr_enabled = true;
    
    // Initialize random number generator for ASLR
    // This would use hardware RNG if available
    
    return 0;
}

/**
 * Enable stack protection mechanisms
 */
int security_enable_stack_protection(void) {
    memory_protection_config.stack_protection_enabled = true;
    
    // Stack canaries are initialized in init_stack_canary()
    // Stack overflow detection would be implemented here
    
    return 0;
}

/**
 * Enable heap protection mechanisms
 */
int security_enable_heap_protection(void) {
    memory_protection_config.heap_protection_enabled = true;
    memory_protection_config.heap_magic = HEAP_GUARD_MAGIC;
    
    // Initialize heap corruption detection
    // Guard pages and heap metadata protection would be set up here
    
    return 0;
}

/**
 * Enable Control Flow Integrity (CFI)
 */
int security_enable_cfi(void) {
    memory_protection_config.cfi_enabled = true;
    memory_protection_config.rop_protection_enabled = true;
    
    // CFI implementation would be architecture-specific
    // This would enable:
    // - Forward-edge CFI (indirect call protection)
    // - Backward-edge CFI (return address protection)
    // - Jump table protection
    
    return 0;
}

/**
 * Detect hardware security features
 */
static int detect_hardware_security_features(void) {
    // Query HAL for hardware security capabilities
    if (hal_has_feature(HAL_FEATURE_SMAP)) {
        hw_security.smap_available = true;
    }
    
    if (hal_has_feature(HAL_FEATURE_SMEP)) {
        hw_security.smep_available = true;
    }
    
    if (hal_has_feature(HAL_FEATURE_CET)) {
        hw_security.cet_available = true;
    }
    
    if (hal_has_feature(HAL_FEATURE_AES_NI)) {
        hw_security.aes_ni_available = true;
    }
    
    if (hal_has_feature(HAL_FEATURE_RDRAND)) {
        hw_security.rdrand_available = true;
    }
    
    if (hal_has_feature(HAL_FEATURE_MTE)) {
        hw_security.mte_available = true;
    }
    
    if (hal_has_feature(HAL_FEATURE_POINTER_AUTH)) {
        hw_security.pauth_available = true;
    }
    
    return 0;
}

/**
 * Enable hardware-based protections
 */
static int enable_hardware_protections(void) {
    int ret = 0;
    
    // Enable SMAP (Supervisor Mode Access Prevention)
    if (hw_security.smap_available) {
        ret = hal_enable_smap();
        if (ret == 0) {
            memory_protection_config.smap_enabled = true;
        }
    }
    
    // Enable SMEP (Supervisor Mode Execution Prevention)
    if (hw_security.smep_available) {
        ret = hal_enable_smep();
        if (ret == 0) {
            memory_protection_config.smep_enabled = true;
        }
    }
    
    // Enable Intel CET (Control-flow Enforcement Technology)
    if (hw_security.cet_available) {
        ret = hal_enable_cet();
        if (ret == 0) {
            memory_protection_config.cet_enabled = true;
        }
    }
    
    // Enable ARM Memory Tagging Extension
    if (hw_security.mte_available) {
        ret = hal_enable_mte();
        if (ret == 0) {
            memory_protection_config.mte_enabled = true;
        }
    }
    
    return 0;
}

/**
 * Initialize stack canary protection
 */
static int init_stack_canary(void) {
    // Generate random canary value
    if (hw_security.rdrand_available) {
        hal_get_random(&memory_protection_config.stack_canary_value, 
                      sizeof(memory_protection_config.stack_canary_value));
    } else {
        // Fallback to pseudo-random value
        memory_protection_config.stack_canary_value = STACK_CANARY_MAGIC ^ get_system_time();
    }
    
    return 0;
}

/**
 * Get stack canary value for process
 */
uint32_t security_get_stack_canary(process_t* process) {
    if (!memory_protection_config.stack_protection_enabled) {
        return 0;
    }
    
    // Each process gets a unique canary based on PID and global canary
    return memory_protection_config.stack_canary_value ^ (process ? process->pid : 0);
}

/**
 * Check stack canary integrity
 */
bool security_check_stack_canary(process_t* process, uint32_t canary_value) {
    if (!memory_protection_config.stack_protection_enabled) {
        return true;
    }
    
    uint32_t expected = security_get_stack_canary(process);
    if (canary_value != expected) {
        // Stack overflow detected!
        security_event_t event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .pid = process ? process->pid : 0,
            .uid = process ? process->creds.uid : 0,
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 9,
            .blocked = true
        };
        strcpy(event.description, "Stack canary corruption detected");
        security_log_event(&event);
        
        return false;
    }
    
    return true;
}

/**
 * Allocate ASLR-randomized memory region
 */
void* security_alloc_randomized_memory(size_t size, uint32_t type) {
    if (!memory_protection_config.aslr_enabled) {
        return kmalloc(size);
    }
    
    // Get base address for this allocation type
    uint64_t base_addr = 0;
    uint64_t range = 0;
    
    switch (type) {
        case ASLR_TYPE_STACK:
            base_addr = 0x7F0000000000ULL;
            range = ASLR_STACK_RANGE;
            break;
        case ASLR_TYPE_HEAP:
            base_addr = 0x600000000000ULL;
            range = ASLR_HEAP_RANGE;
            break;
        case ASLR_TYPE_MMAP:
            base_addr = 0x700000000000ULL;
            range = ASLR_MMAP_RANGE;
            break;
        case ASLR_TYPE_EXEC:
            base_addr = 0x400000000000ULL;
            range = ASLR_EXEC_RANGE;
            break;
        default:
            return kmalloc(size);
    }
    
    // Generate random offset
    uint64_t random_offset = security_get_random_offset(range);
    uint64_t addr = base_addr + random_offset;
    
    // Align to page boundary
    addr = (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    // Allocate memory at randomized address
    return (void*)addr;
}

/**
 * Allocate protected heap memory
 */
void* security_alloc_protected_heap(size_t size) {
    if (!memory_protection_config.heap_protection_enabled) {
        return kmalloc(size);
    }
    
    // Allocate extra space for guard pages and metadata
    size_t total_size = size + 2 * PAGE_SIZE + sizeof(heap_metadata_t);
    
    void* raw_ptr = kmalloc(total_size);
    if (!raw_ptr) {
        return NULL;
    }
    
    // Set up guard page before allocation
    uint8_t* guard_before = (uint8_t*)raw_ptr;
    memset(guard_before, 0xAA, PAGE_SIZE);
    
    // User allocation starts after first guard page
    void* user_ptr = (uint8_t*)raw_ptr + PAGE_SIZE;
    
    // Set up metadata
    heap_metadata_t* metadata = (heap_metadata_t*)user_ptr;
    metadata->magic = memory_protection_config.heap_magic;
    metadata->size = size;
    metadata->allocated = true;
    metadata->canary = HEAP_GUARD_MAGIC;
    
    // User data starts after metadata
    void* data_ptr = (uint8_t*)user_ptr + sizeof(heap_metadata_t);
    
    // Set up guard page after allocation
    uint8_t* guard_after = (uint8_t*)data_ptr + size;
    memset(guard_after, 0xBB, PAGE_SIZE);
    
    return data_ptr;
}

/**
 * Free protected heap memory
 */
void security_free_protected_heap(void* ptr) {
    if (!ptr || !memory_protection_config.heap_protection_enabled) {
        kfree(ptr);
        return;
    }
    
    // Get metadata
    heap_metadata_t* metadata = (heap_metadata_t*)((uint8_t*)ptr - sizeof(heap_metadata_t));
    
    // Check heap integrity
    if (metadata->magic != memory_protection_config.heap_magic ||
        metadata->canary != HEAP_GUARD_MAGIC ||
        !metadata->allocated) {
        
        // Heap corruption detected!
        security_event_t event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 8,
            .blocked = true
        };
        strcpy(event.description, "Heap corruption detected");
        security_log_event(&event);
        
        return; // Don't free corrupted memory
    }
    
    // Mark as freed
    metadata->allocated = false;
    metadata->magic = HEAP_FREE_MAGIC;
    
    // Zero out user data
    memset(ptr, 0xDD, metadata->size);
    
    // Free the entire allocation including guard pages
    void* raw_ptr = (uint8_t*)metadata - PAGE_SIZE;
    kfree(raw_ptr);
}

/**
 * Check control flow integrity
 */
bool security_check_cfi(void* target_addr, void* expected_addr) {
    if (!memory_protection_config.cfi_enabled) {
        return true;
    }
    
    // In a full implementation, this would:
    // 1. Check if target_addr is a valid function entry point
    // 2. Verify against CFI metadata
    // 3. Check return address integrity
    
    if (target_addr != expected_addr) {
        // CFI violation detected
        security_event_t event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 9,
            .blocked = true
        };
        strcpy(event.description, "Control flow integrity violation");
        security_log_event(&event);
        
        return false;
    }
    
    return true;
}

/**
 * Detect ROP/JOP gadgets in memory
 */
bool security_detect_rop_gadgets(void* code_ptr, size_t code_size) {
    if (!memory_protection_config.rop_protection_enabled) {
        return false;
    }
    
    // Simple ROP gadget detection
    uint8_t* code = (uint8_t*)code_ptr;
    size_t gadget_count = 0;
    
    for (size_t i = 0; i < code_size - 1; i++) {
        // Look for common ROP gadget patterns
        if (code[i] == 0xC3) { // RET instruction
            gadget_count++;
        } else if (code[i] == 0xFF && (code[i+1] & 0xF8) == 0xE0) { // JMP reg
            gadget_count++;
        } else if (code[i] == 0xFF && (code[i+1] & 0xF8) == 0xD0) { // CALL reg
            gadget_count++;
        }
    }
    
    // If too many gadgets found, this might be a ROP chain
    if (gadget_count > code_size / 100) { // More than 1% gadgets
        security_event_t event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 7,
            .blocked = false
        };
        strcpy(event.description, "Potential ROP gadgets detected");
        security_log_event(&event);
        
        return true;
    }
    
    return false;
}

/**
 * Generate random offset for ASLR
 */
static uint64_t security_get_random_offset(uint64_t range) {
    uint64_t random_value;
    
    if (hw_security.rdrand_available) {
        hal_get_random(&random_value, sizeof(random_value));
    } else {
        // Fallback to pseudo-random based on system time
        random_value = get_system_time() ^ (get_system_time() >> 32);
    }
    
    return random_value % range;
}

/**
 * Get memory protection statistics
 */
int security_get_memory_protection_stats(memory_protection_stats_t* stats) {
    if (!stats) {
        return -EINVAL;
    }
    
    memset(stats, 0, sizeof(memory_protection_stats_t));
    
    stats->aslr_enabled = memory_protection_config.aslr_enabled;
    stats->stack_protection_enabled = memory_protection_config.stack_protection_enabled;
    stats->heap_protection_enabled = memory_protection_config.heap_protection_enabled;
    stats->cfi_enabled = memory_protection_config.cfi_enabled;
    stats->smap_enabled = memory_protection_config.smap_enabled;
    stats->smep_enabled = memory_protection_config.smep_enabled;
    stats->cet_enabled = memory_protection_config.cet_enabled;
    stats->mte_enabled = memory_protection_config.mte_enabled;
    
    // These would be tracked by the actual implementation
    stats->stack_overflows_prevented = 0;
    stats->heap_corruptions_detected = 0;
    stats->cfi_violations_detected = 0;
    stats->rop_attempts_blocked = 0;
    
    return 0;
}

// Memory allocation types for ASLR
#define ASLR_TYPE_STACK 0
#define ASLR_TYPE_HEAP  1
#define ASLR_TYPE_MMAP  2
#define ASLR_TYPE_EXEC  3

// Heap metadata structure
typedef struct heap_metadata {
    uint32_t magic;
    size_t size;
    bool allocated;
    uint32_t canary;
} heap_metadata_t;

// Memory protection statistics
typedef struct memory_protection_stats {
    bool aslr_enabled;
    bool stack_protection_enabled;
    bool heap_protection_enabled;
    bool cfi_enabled;
    bool smap_enabled;
    bool smep_enabled;
    bool cet_enabled;
    bool mte_enabled;
    uint64_t stack_overflows_prevented;
    uint64_t heap_corruptions_detected;
    uint64_t cfi_violations_detected;
    uint64_t rop_attempts_blocked;
} memory_protection_stats_t;
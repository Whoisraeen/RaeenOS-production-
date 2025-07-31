/**
 * @file hal_x86_64.h
 * @brief x86-64 Hardware Abstraction Layer Header
 * 
 * This header defines x86-64 specific structures, constants, and function
 * prototypes for the hardware abstraction layer.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#ifndef HAL_X86_64_H
#define HAL_X86_64_H

#include "../../include/hal_interface.h"
#include "../../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// x86-64 specific constants
#define X86_64_MAX_CPUS         256
#define X86_64_PAGE_SHIFT       12
#define X86_64_PAGE_SIZE        (1UL << X86_64_PAGE_SHIFT)
#define X86_64_PAGE_MASK        (~(X86_64_PAGE_SIZE - 1))

// CPU feature bits
#define X86_64_FEATURE_FPU      (1 << 0)
#define X86_64_FEATURE_SSE      (1 << 1)
#define X86_64_FEATURE_SSE2     (1 << 2)
#define X86_64_FEATURE_SSE3     (1 << 3)
#define X86_64_FEATURE_SSSE3    (1 << 4)
#define X86_64_FEATURE_SSE4_1   (1 << 5)
#define X86_64_FEATURE_SSE4_2   (1 << 6)
#define X86_64_FEATURE_AVX      (1 << 7)
#define X86_64_FEATURE_AVX2     (1 << 8)
#define X86_64_FEATURE_AES      (1 << 9)
#define X86_64_FEATURE_RDRAND   (1 << 10)
#define X86_64_FEATURE_BMI1     (1 << 11)
#define X86_64_FEATURE_BMI2     (1 << 12)
#define X86_64_FEATURE_ADX      (1 << 13)
#define X86_64_FEATURE_SHA      (1 << 14)

// MSR definitions
#define MSR_IA32_APIC_BASE      0x1B
#define MSR_IA32_TSC            0x10
#define MSR_IA32_MTRR_CAP       0xFE
#define MSR_IA32_MTRR_DEF_TYPE  0x2FF
#define MSR_IA32_PAT            0x277
#define MSR_IA32_EFER           0xC0000080
#define MSR_IA32_STAR           0xC0000081
#define MSR_IA32_LSTAR          0xC0000082
#define MSR_IA32_CSTAR          0xC0000083
#define MSR_IA32_SF_MASK        0xC0000084
#define MSR_IA32_FS_BASE        0xC0000100
#define MSR_IA32_GS_BASE        0xC0000101
#define MSR_IA32_KERNEL_GS_BASE 0xC0000102

// EFER bits
#define EFER_SCE    (1 << 0)   // System Call Extensions
#define EFER_LME    (1 << 8)   // Long Mode Enable
#define EFER_LMA    (1 << 10)  // Long Mode Active
#define EFER_NXE    (1 << 11)  // No-Execute Enable

// CR0 bits
#define CR0_PE      (1 << 0)   // Protected Mode Enable
#define CR0_MP      (1 << 1)   // Monitor Coprocessor
#define CR0_EM      (1 << 2)   // Emulation
#define CR0_TS      (1 << 3)   // Task Switched
#define CR0_ET      (1 << 4)   // Extension Type
#define CR0_NE      (1 << 5)   // Numeric Error
#define CR0_WP      (1 << 16)  // Write Protect
#define CR0_AM      (1 << 18)  // Alignment Mask
#define CR0_NW      (1 << 29)  // Not Write-through
#define CR0_CD      (1 << 30)  // Cache Disable
#define CR0_PG      (1 << 31)  // Paging

// CR4 bits
#define CR4_VME     (1 << 0)   // Virtual-8086 Mode Extensions
#define CR4_PVI     (1 << 1)   // Protected Mode Virtual Interrupts
#define CR4_TSD     (1 << 2)   // Time Stamp Disable
#define CR4_DE      (1 << 3)   // Debugging Extensions
#define CR4_PSE     (1 << 4)   // Page Size Extensions
#define CR4_PAE     (1 << 5)   // Physical Address Extension
#define CR4_MCE     (1 << 6)   // Machine Check Enable
#define CR4_PGE     (1 << 7)   // Page Global Enable
#define CR4_PCE     (1 << 8)   // Performance Counter Enable
#define CR4_OSFXSR  (1 << 9)   // OS FXSAVE/FXRSTOR Support
#define CR4_OSXMMEXCPT (1 << 10) // OS Unmasked Exception Support
#define CR4_UMIP    (1 << 11)  // User-Mode Instruction Prevention
#define CR4_VMXE    (1 << 13)  // VMX Enable
#define CR4_SMXE    (1 << 14)  // SMX Enable
#define CR4_FSGSBASE (1 << 16) // FS/GS Base Access
#define CR4_PCIDE   (1 << 17)  // Process Context ID Enable
#define CR4_OSXSAVE (1 << 18)  // OS XSAVE Support
#define CR4_SMEP    (1 << 20)  // Supervisor Mode Execution Prevention
#define CR4_SMAP    (1 << 21)  // Supervisor Mode Access Prevention

// x86-64 specific CPU information structure
typedef struct {
    uint32_t vendor_id[4];     // CPU vendor identification
    uint32_t brand_string[12]; // CPU brand string
    uint32_t family;           // CPU family
    uint32_t model;            // CPU model
    uint32_t stepping;         // CPU stepping
    uint32_t features;         // Feature flags
    uint32_t extended_features; // Extended feature flags
    uint32_t cache_info[4];    // Cache information
    uint64_t tsc_frequency;    // TSC frequency in Hz
    bool apic_available;       // Local APIC available
    bool x2apic_available;     // x2APIC available
    uint8_t apic_id;          // Local APIC ID
} x86_64_cpu_info_t;

// x86-64 memory type range register (MTRR) structure
typedef struct {
    uint64_t base;
    uint64_t mask;
    uint8_t type;
    bool valid;
} x86_64_mtrr_t;

// x86-64 NUMA node information
typedef struct {
    uint32_t node_id;
    uint64_t memory_start;
    uint64_t memory_size;
    uint32_t cpu_mask;         // Bitmask of CPUs in this node
    uint32_t distance[32];     // Distance to other nodes
} x86_64_numa_node_t;

// x86-64 platform-specific state
typedef struct {
    x86_64_cpu_info_t cpu_info;
    x86_64_mtrr_t mtrr[8];     // Variable MTRRs
    x86_64_numa_node_t numa_nodes[64];
    uint32_t numa_node_count;
    uint64_t memory_map_entries;
    hal_memory_region_t* memory_map;
    bool acpi_available;
    void* acpi_tables;
    bool smp_enabled;
    uint32_t active_cpus;
} x86_64_platform_data_t;

// Function prototypes for external assembly functions
extern void x86_64_cpu_pause(void);
extern void x86_64_memory_barrier(void);
extern uint64_t x86_64_read_tsc(void);
extern uint64_t x86_64_read_msr(uint32_t msr);
extern void x86_64_write_msr(uint32_t msr, uint64_t value);
extern void x86_64_cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
extern void x86_64_cpuid_count(uint32_t leaf, uint32_t count, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
extern void x86_64_wbinvd(void);
extern void x86_64_invlpg(void* addr);
extern void x86_64_flush_tlb(void);
extern void x86_64_flush_tlb_single(void* addr);
extern uint64_t x86_64_read_cr0(void);
extern void x86_64_write_cr0(uint64_t value);
extern uint64_t x86_64_read_cr2(void);
extern uint64_t x86_64_read_cr3(void);
extern void x86_64_write_cr3(uint64_t value);
extern uint64_t x86_64_read_cr4(void);
extern void x86_64_write_cr4(uint64_t value);
extern uint64_t x86_64_read_dr0(void);
extern uint64_t x86_64_read_dr1(void);
extern uint64_t x86_64_read_dr2(void);
extern uint64_t x86_64_read_dr3(void);
extern uint64_t x86_64_read_dr6(void);
extern uint64_t x86_64_read_dr7(void);
extern void x86_64_write_dr0(uint64_t value);
extern void x86_64_write_dr1(uint64_t value);
extern void x86_64_write_dr2(uint64_t value);
extern void x86_64_write_dr3(uint64_t value);
extern void x86_64_write_dr6(uint64_t value);
extern void x86_64_write_dr7(uint64_t value);

// x86-64 HAL specific functions
int hal_x86_64_init(hal_operations_t** ops);
int x86_64_detect_cpu_features(x86_64_cpu_info_t* info);
int x86_64_setup_mtrr(void);
int x86_64_detect_numa_topology(void);
int x86_64_calibrate_timers(void);
int x86_64_init_acpi(void);
int x86_64_setup_smp(void);

// Cache management functions
void x86_64_cache_enable(void);
void x86_64_cache_disable(void);
int x86_64_get_cache_info(uint32_t level, uint32_t* size, uint32_t* line_size, uint32_t* ways);

// Performance monitoring functions
int x86_64_setup_performance_counters(void);
uint64_t x86_64_read_performance_counter(uint32_t counter);
void x86_64_write_performance_counter(uint32_t counter, uint64_t value);

// Power management functions
int x86_64_setup_power_management(void);
int x86_64_set_cpu_frequency(uint32_t cpu_id, uint32_t frequency);
uint32_t x86_64_get_cpu_frequency(uint32_t cpu_id);
int x86_64_enter_sleep_state(uint32_t state);

// Virtualization support functions
bool x86_64_vmx_supported(void);
bool x86_64_svm_supported(void);
int x86_64_enable_virtualization(void);
int x86_64_disable_virtualization(void);

// Security feature functions
int x86_64_setup_smep_smap(void);
int x86_64_setup_control_flow_integrity(void);
bool x86_64_has_intel_cet(void);
bool x86_64_has_amd_cet(void);

// Debugging and profiling functions
int x86_64_setup_hardware_breakpoints(void);
int x86_64_set_hardware_breakpoint(uint32_t index, void* addr, uint32_t type, uint32_t len);
int x86_64_clear_hardware_breakpoint(uint32_t index);
int x86_64_setup_branch_tracing(void);

// Utility macros
#define X86_64_ALIGN_UP(addr, align)    (((addr) + (align) - 1) & ~((align) - 1))
#define X86_64_ALIGN_DOWN(addr, align)  ((addr) & ~((align) - 1))
#define X86_64_IS_ALIGNED(addr, align)  (((addr) & ((align) - 1)) == 0)

#define X86_64_PAGE_ALIGN_UP(addr)      X86_64_ALIGN_UP(addr, X86_64_PAGE_SIZE)
#define X86_64_PAGE_ALIGN_DOWN(addr)    X86_64_ALIGN_DOWN(addr, X86_64_PAGE_SIZE)
#define X86_64_IS_PAGE_ALIGNED(addr)    X86_64_IS_ALIGNED(addr, X86_64_PAGE_SIZE)

// CPU identification macros
#define X86_64_VENDOR_INTEL     0x756E6547  // "Genu"
#define X86_64_VENDOR_AMD       0x68747541  // "Auth"
#define X86_64_VENDOR_VIA       0x746E6543  // "Cent"

#ifdef __cplusplus
}
#endif

#endif // HAL_X86_64_H
/**
 * @file hal_arm64.h
 * @brief ARM64 Hardware Abstraction Layer Header
 * 
 * This header defines ARM64 specific structures, constants, and function
 * prototypes for the hardware abstraction layer.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#ifndef HAL_ARM64_H
#define HAL_ARM64_H

#include "../../include/hal_interface.h"
#include "../../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ARM64 specific constants
#define ARM64_MAX_CPUS          256
#define ARM64_PAGE_SHIFT        12
#define ARM64_PAGE_SIZE         (1UL << ARM64_PAGE_SHIFT)
#define ARM64_PAGE_MASK         (~(ARM64_PAGE_SIZE - 1))

// System register encodings
#define ARM64_MIDR_EL1          0x30000000  // Main ID Register
#define ARM64_MPIDR_EL1         0x30000005  // Multiprocessor Affinity Register
#define ARM64_ID_AA64PFR0_EL1   0x30000040  // Processor Feature Register 0
#define ARM64_ID_AA64PFR1_EL1   0x30000041  // Processor Feature Register 1
#define ARM64_ID_AA64DFR0_EL1   0x30000050  // Debug Feature Register 0
#define ARM64_ID_AA64DFR1_EL1   0x30000051  // Debug Feature Register 1
#define ARM64_ID_AA64AFR0_EL1   0x30000054  // Auxiliary Feature Register 0
#define ARM64_ID_AA64AFR1_EL1   0x30000055  // Auxiliary Feature Register 1
#define ARM64_ID_AA64ISAR0_EL1  0x30000060  // Instruction Set Attribute Register 0
#define ARM64_ID_AA64ISAR1_EL1  0x30000061  // Instruction Set Attribute Register 1
#define ARM64_ID_AA64MMFR0_EL1  0x30000070  // Memory Model Feature Register 0
#define ARM64_ID_AA64MMFR1_EL1  0x30000071  // Memory Model Feature Register 1
#define ARM64_ID_AA64MMFR2_EL1  0x30000072  // Memory Model Feature Register 2

#define ARM64_SCTLR_EL1         0x30001000  // System Control Register
#define ARM64_ACTLR_EL1         0x30001001  // Auxiliary Control Register
#define ARM64_CPACR_EL1         0x30001002  // Coprocessor Access Control Register

#define ARM64_TTBR0_EL1         0x30002000  // Translation Table Base Register 0
#define ARM64_TTBR1_EL1         0x30002001  // Translation Table Base Register 1
#define ARM64_TCR_EL1           0x30002002  // Translation Control Register
#define ARM64_MAIR_EL1          0x30002510  // Memory Attribute Indirection Register

#define ARM64_VBAR_EL1          0x3000C000  // Vector Base Address Register
#define ARM64_ESR_EL1           0x30005200  // Exception Syndrome Register
#define ARM64_FAR_EL1           0x30006000  // Fault Address Register
#define ARM64_PAR_EL1           0x30007400  // Physical Address Register

#define ARM64_CNTKCTL_EL1       0x30001008  // Counter-timer Kernel Control Register
#define ARM64_CNTPCT_EL0        0x3B9D200E  // Counter-timer Physical Count Register
#define ARM64_CNTFRQ_EL0        0x3B9D0000  // Counter-timer Frequency Register

// Exception level definitions
#define ARM64_EL0               0
#define ARM64_EL1               1
#define ARM64_EL2               2
#define ARM64_EL3               3

// SCTLR_EL1 bits
#define SCTLR_EL1_M             (1 << 0)    // MMU enable
#define SCTLR_EL1_A             (1 << 1)    // Alignment check enable
#define SCTLR_EL1_C             (1 << 2)    // Data cache enable
#define SCTLR_EL1_SA            (1 << 3)    // Stack alignment check enable
#define SCTLR_EL1_SA0           (1 << 4)    // Stack alignment check enable for EL0
#define SCTLR_EL1_CP15BEN       (1 << 5)    // CP15 barrier enable
#define SCTLR_EL1_ITD           (1 << 7)    // IT disable
#define SCTLR_EL1_SED           (1 << 8)    // SETEND disable
#define SCTLR_EL1_UMA           (1 << 9)    // User mask access
#define SCTLR_EL1_I             (1 << 12)   // Instruction cache enable
#define SCTLR_EL1_DZE           (1 << 14)   // DZ (Data Zero) enable
#define SCTLR_EL1_UCT           (1 << 15)   // User cache type register access
#define SCTLR_EL1_nTWI          (1 << 16)   // Not trap WFI
#define SCTLR_EL1_nTWE          (1 << 18)   // Not trap WFE
#define SCTLR_EL1_WXN           (1 << 19)   // Write permission implies XN
#define SCTLR_EL1_E0E           (1 << 24)   // Exception endianness for EL0
#define SCTLR_EL1_EE            (1 << 25)   // Exception endianness
#define SCTLR_EL1_UCI           (1 << 26)   // User cache instruction access
#define SCTLR_EL1_EnDA          (1 << 27)   // Enable pointer authentication
#define SCTLR_EL1_EnDB          (1 << 30)   // Enable pointer authentication

// TCR_EL1 bits
#define TCR_EL1_T0SZ_SHIFT      0
#define TCR_EL1_EPD0            (1 << 7)    // Translation table walk disable for TTBR0
#define TCR_EL1_IRGN0_SHIFT     8
#define TCR_EL1_ORGN0_SHIFT     10
#define TCR_EL1_SH0_SHIFT       12
#define TCR_EL1_TG0_SHIFT       14
#define TCR_EL1_T1SZ_SHIFT      16
#define TCR_EL1_A1              (1 << 22)   // ASID select
#define TCR_EL1_EPD1            (1 << 23)   // Translation table walk disable for TTBR1
#define TCR_EL1_IRGN1_SHIFT     24
#define TCR_EL1_ORGN1_SHIFT     26
#define TCR_EL1_SH1_SHIFT       28
#define TCR_EL1_TG1_SHIFT       30
#define TCR_EL1_IPS_SHIFT       32
#define TCR_EL1_AS              (1ULL << 36) // ASID size
#define TCR_EL1_TBI0            (1ULL << 37) // Top byte ignored for TTBR0
#define TCR_EL1_TBI1            (1ULL << 38) // Top byte ignored for TTBR1

// Memory attribute values for MAIR_EL1
#define ARM64_MAIR_DEVICE_nGnRnE    0x00    // Device non-gathering, non-reordering, no early write ack
#define ARM64_MAIR_DEVICE_nGnRE     0x04    // Device non-gathering, non-reordering, early write ack
#define ARM64_MAIR_DEVICE_nGRE      0x08    // Device non-gathering, reordering, early write ack
#define ARM64_MAIR_DEVICE_GRE       0x0C    // Device gathering, reordering, early write ack
#define ARM64_MAIR_NORMAL_NC        0x44    // Normal memory, non-cacheable
#define ARM64_MAIR_NORMAL_WT        0xBB    // Normal memory, write-through cacheable
#define ARM64_MAIR_NORMAL_WB        0xFF    // Normal memory, write-back cacheable

// Page table entry bits
#define ARM64_PTE_VALID             (1ULL << 0)
#define ARM64_PTE_TYPE_MASK         (3ULL << 0)
#define ARM64_PTE_TYPE_FAULT        (0ULL << 0)
#define ARM64_PTE_TYPE_PAGE         (3ULL << 0)
#define ARM64_PTE_TYPE_BLOCK        (1ULL << 0)
#define ARM64_PTE_ATTRINDX_SHIFT    2
#define ARM64_PTE_NS                (1ULL << 5)    // Non-secure
#define ARM64_PTE_AP_SHIFT          6
#define ARM64_PTE_AP_RW_EL1         (0ULL << 6)    // Read/write at EL1
#define ARM64_PTE_AP_RW_ALL         (1ULL << 6)    // Read/write at EL1 and EL0
#define ARM64_PTE_AP_RO_EL1         (2ULL << 6)    // Read-only at EL1
#define ARM64_PTE_AP_RO_ALL         (3ULL << 6)    // Read-only at EL1 and EL0
#define ARM64_PTE_SH_SHIFT          8
#define ARM64_PTE_AF                (1ULL << 10)   // Access flag
#define ARM64_PTE_nG                (1ULL << 11)   // Not global
#define ARM64_PTE_ADDR_MASK         0x0000FFFFFFFFF000ULL
#define ARM64_PTE_PXN               (1ULL << 53)   // Privileged execute never
#define ARM64_PTE_UXN               (1ULL << 54)   // Unprivileged execute never

// Cache operation types
#define ARM64_CACHE_OP_CLEAN        0
#define ARM64_CACHE_OP_INVALIDATE   1
#define ARM64_CACHE_OP_CLEAN_INV    2

// ARM64 CPU information structure
typedef struct {
    uint32_t implementer;      // CPU implementer ID
    uint32_t variant;          // CPU variant
    uint32_t architecture;     // Architecture version
    uint32_t part_number;      // CPU part number
    uint32_t revision;         // CPU revision
    uint64_t features;         // Feature flags
    uint32_t cache_info[16];   // Cache hierarchy information
    bool sve_available;        // Scalable Vector Extension available
    bool sve2_available;       // SVE2 available
    bool pointer_auth_available; // Pointer authentication available
    bool mte_available;        // Memory Tagging Extension available
} arm64_cpu_info_t;

// ARM64 GIC (Generic Interrupt Controller) information
typedef struct {
    uint32_t version;          // GIC version (2, 3, or 4)
    phys_addr_t dist_base;     // Distributor base address
    phys_addr_t cpu_base;      // CPU interface base address (GICv2)
    phys_addr_t redist_base;   // Redistributor base address (GICv3+)
    uint32_t max_irqs;         // Maximum number of IRQs
    uint32_t max_cpus;         // Maximum number of CPUs
} arm64_gic_info_t;

// ARM64 NUMA node information
typedef struct {
    uint32_t node_id;
    uint64_t memory_start;
    uint64_t memory_size;
    uint32_t cpu_mask[8];      // Bitmask of CPUs in this node (up to 256 CPUs)
    uint32_t distance[64];     // Distance to other nodes
} arm64_numa_node_t;

// ARM64 platform-specific state
typedef struct {
    arm64_cpu_info_t cpu_info;
    arm64_gic_info_t gic_info;
    arm64_numa_node_t numa_nodes[64];
    uint32_t numa_node_count;
    void* device_tree_base;
    uint64_t memory_map_entries;
    hal_memory_region_t* memory_map;
    bool psci_available;
    uint32_t psci_version;
    bool secure_monitor_available;
    uint32_t exception_level;
} arm64_platform_data_t;

// Function prototypes for external assembly functions
extern uint64_t arm64_read_sysreg(uint32_t reg);
extern void arm64_write_sysreg(uint32_t reg, uint64_t value);
extern void arm64_isb(void);
extern void arm64_dsb(void);
extern void arm64_dmb(void);
extern void arm64_wfi(void);
extern void arm64_wfe(void);
extern void arm64_sev(void);
extern void arm64_sevl(void);
extern uint64_t arm64_read_cntpct_el0(void);
extern uint64_t arm64_read_cntfrq_el0(void);
extern void arm64_dc_civac(void* addr);
extern void arm64_dc_cvac(void* addr);
extern void arm64_dc_cvau(void* addr);
extern void arm64_dc_ivac(void* addr);
extern void arm64_ic_iallu(void);
extern void arm64_ic_ialluis(void);
extern void arm64_ic_ivau(void* addr);
extern void arm64_tlbi_vmalle1(void);
extern void arm64_tlbi_vmalle1is(void);
extern void arm64_tlbi_vae1(uint64_t addr);
extern void arm64_tlbi_vae1is(uint64_t addr);
extern void arm64_at_s1e1r(void* addr);
extern void arm64_at_s1e1w(void* addr);
extern uint64_t arm64_read_par_el1(void);

// ARM64 HAL specific functions
int hal_arm64_init(hal_operations_t** ops);
int arm64_detect_cpu_features(arm64_cpu_info_t* info);
int arm64_setup_gic(arm64_gic_info_t* gic_info);
int arm64_detect_numa_topology(void);
int arm64_init_device_tree(void* dt_base);
int arm64_setup_psci(void);

// Cache management functions
void arm64_cache_enable(void);
void arm64_cache_disable(void);
int arm64_get_cache_info(uint32_t level, uint32_t* size, uint32_t* line_size, uint32_t* ways);
void arm64_cache_op_range(void* start, size_t size, int op);

// Performance monitoring functions
int arm64_setup_performance_counters(void);
uint64_t arm64_read_performance_counter(uint32_t counter);
void arm64_write_performance_counter(uint32_t counter, uint64_t value);

// Security feature functions
bool arm64_has_pointer_auth(void);
bool arm64_has_mte(void);
bool arm64_has_sve(void);
bool arm64_has_sve2(void);
int arm64_enable_pointer_auth(void);
int arm64_enable_mte(void);

// Virtualization support functions
bool arm64_has_virtualization(void);
uint32_t arm64_get_exception_level(void);
int arm64_setup_stage2_translation(void);

// Device tree functions
int arm64_dt_parse(void* dt_base);
void* arm64_dt_find_node(const char* path);
void* arm64_dt_get_property(const void* node, const char* name, int* len);
uint64_t arm64_dt_get_address(const void* node, int index);
uint32_t arm64_dt_get_interrupt(const void* node, int index);

// PSCI (Power State Coordination Interface) functions
int arm64_psci_cpu_on(uint64_t mpidr, uint64_t entry_point, uint64_t context_id);
int arm64_psci_cpu_off(void);
int arm64_psci_cpu_suspend(uint32_t power_state, uint64_t entry_point, uint64_t context_id);
int arm64_psci_system_off(void);
int arm64_psci_system_reset(void);
uint32_t arm64_psci_get_version(void);

// Memory management helper functions
int arm64_setup_mmu(void);
int arm64_create_page_table(uint64_t* table, uint64_t virt, uint64_t phys, size_t size, uint64_t attrs);
void arm64_invalidate_tlb(void);
void arm64_invalidate_tlb_range(void* start, size_t size);

// Interrupt handling functions
int arm64_setup_exception_vectors(void);
void arm64_enable_irq(void);
void arm64_disable_irq(void);
void arm64_enable_fiq(void);
void arm64_disable_fiq(void);

// Debug and tracing functions
int arm64_setup_hardware_breakpoints(void);
int arm64_set_hardware_breakpoint(uint32_t index, void* addr, uint32_t type);
int arm64_clear_hardware_breakpoint(uint32_t index);
int arm64_setup_etm_tracing(void);

// Atomic operations
uint64_t arm64_atomic_cmpxchg64(volatile uint64_t* ptr, uint64_t old, uint64_t new);
uint64_t arm64_atomic_add64(volatile uint64_t* ptr, uint64_t value);
uint64_t arm64_atomic_sub64(volatile uint64_t* ptr, uint64_t value);
void arm64_atomic_inc64(volatile uint64_t* ptr);
void arm64_atomic_dec64(volatile uint64_t* ptr);

// Utility macros
#define ARM64_ALIGN_UP(addr, align)    (((addr) + (align) - 1) & ~((align) - 1))
#define ARM64_ALIGN_DOWN(addr, align)  ((addr) & ~((align) - 1))
#define ARM64_IS_ALIGNED(addr, align)  (((addr) & ((align) - 1)) == 0)

#define ARM64_PAGE_ALIGN_UP(addr)      ARM64_ALIGN_UP(addr, ARM64_PAGE_SIZE)
#define ARM64_PAGE_ALIGN_DOWN(addr)    ARM64_ALIGN_DOWN(addr, ARM64_PAGE_SIZE)
#define ARM64_IS_PAGE_ALIGNED(addr)    ARM64_IS_ALIGNED(addr, ARM64_PAGE_SIZE)

// CPU implementer IDs
#define ARM64_IMPLEMENTER_ARM       0x41
#define ARM64_IMPLEMENTER_BROADCOM  0x42
#define ARM64_IMPLEMENTER_CAVIUM    0x43
#define ARM64_IMPLEMENTER_DEC       0x44
#define ARM64_IMPLEMENTER_FUJITSU   0x46
#define ARM64_IMPLEMENTER_INFINEON  0x49
#define ARM64_IMPLEMENTER_FREESCALE 0x4D
#define ARM64_IMPLEMENTER_NVIDIA    0x4E
#define ARM64_IMPLEMENTER_APM       0x50
#define ARM64_IMPLEMENTER_QUALCOMM  0x51
#define ARM64_IMPLEMENTER_SAMSUNG   0x53
#define ARM64_IMPLEMENTER_MARVELL   0x56
#define ARM64_IMPLEMENTER_APPLE     0x61
#define ARM64_IMPLEMENTER_HISILICON 0x48

#ifdef __cplusplus
}
#endif

#endif // HAL_ARM64_H
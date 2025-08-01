#ifndef INTERRUPT_PRODUCTION_H
#define INTERRUPT_PRODUCTION_H

/**
 * @file interrupt_production.h
 * @brief Production-Grade Interrupt Management System for RaeenOS
 * 
 * This implements a comprehensive interrupt handling system with:
 * - x86-64 Interrupt Descriptor Table (IDT) management
 * - Advanced Programmable Interrupt Controller (APIC) support
 * - Fast interrupt handling and dispatch
 * - Interrupt statistics and profiling
 * - System call interface with security validation
 * - Exception handling with detailed debugging
 * - Performance counters and latency tracking
 * - Interrupt load balancing and affinity
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/types.h"
// Using types.h for kernel build
// Using types.h for kernel build
#include "types.h"
#include "memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// x86-64 interrupt vector assignments
#define IDT_ENTRIES                 256

// CPU exceptions (0-31)
#define INT_DIVIDE_ERROR            0   // #DE - Divide by zero
#define INT_DEBUG                   1   // #DB - Debug exception
#define INT_NMI                     2   // Non-maskable interrupt
#define INT_BREAKPOINT              3   // #BP - Breakpoint
#define INT_OVERFLOW                4   // #OF - Overflow
#define INT_BOUND_RANGE             5   // #BR - Bound range exceeded
#define INT_INVALID_OPCODE          6   // #UD - Invalid opcode
#define INT_DEVICE_NOT_AVAILABLE    7   // #NM - Device not available
#define INT_DOUBLE_FAULT            8   // #DF - Double fault
#define INT_INVALID_TSS             10  // #TS - Invalid TSS
#define INT_SEGMENT_NOT_PRESENT     11  // #NP - Segment not present
#define INT_STACK_FAULT             12  // #SS - Stack segment fault
#define INT_GENERAL_PROTECTION      13  // #GP - General protection fault
#define INT_PAGE_FAULT              14  // #PF - Page fault
#define INT_X87_FPU_ERROR           16  // #MF - x87 FPU error
#define INT_ALIGNMENT_CHECK         17  // #AC - Alignment check
#define INT_MACHINE_CHECK           18  // #MC - Machine check
#define INT_SIMD_FPU_ERROR          19  // #XM - SIMD FPU error
#define INT_VIRTUALIZATION          20  // #VE - Virtualization exception
#define INT_CONTROL_PROTECTION      21  // #CP - Control protection exception

// System interrupts (32-47)
#define INT_TIMER                   32  // System timer
#define INT_KEYBOARD                33  // Keyboard
#define INT_CASCADE                 34  // PIC cascade (not used in APIC)
#define INT_SERIAL_PORT2            35  // Serial port 2
#define INT_SERIAL_PORT1            36  // Serial port 1
#define INT_PARALLEL_PORT2          37  // Parallel port 2
#define INT_FLOPPY                  38  // Floppy disk
#define INT_PARALLEL_PORT1          39  // Parallel port 1
#define INT_RTC                     40  // Real-time clock
#define INT_MOUSE                   44  // PS/2 mouse
#define INT_FPU                     45  // x87 FPU
#define INT_ATA_PRIMARY             46  // Primary ATA
#define INT_ATA_SECONDARY           47  // Secondary ATA

// APIC interrupts (48-255)
#define INT_APIC_TIMER              48  // APIC timer
#define INT_APIC_ERROR              49  // APIC error
#define INT_APIC_SPURIOUS           255 // APIC spurious interrupt

// System call vector
#define INT_SYSCALL                 128 // System call interrupt

// IPI (Inter-Processor Interrupt) vectors
#define IPI_RESCHEDULE              250 // Reschedule IPI
#define IPI_FUNCTION_CALL           251 // Function call IPI
#define IPI_TLB_FLUSH               252 // TLB flush IPI
#define IPI_STOP                    253 // Stop CPU IPI
#define IPI_PANIC                   254 // Panic IPI

// IDT gate types
#define IDT_INTERRUPT_GATE          0x0E
#define IDT_TRAP_GATE              0x0F
#define IDT_CALL_GATE              0x0C
#define IDT_TASK_GATE              0x05

// IDT flags
#define IDT_PRESENT                 0x80
#define IDT_DPL_0                   0x00    // Ring 0
#define IDT_DPL_1                   0x20    // Ring 1
#define IDT_DPL_2                   0x40    // Ring 2
#define IDT_DPL_3                   0x60    // Ring 3

// Interrupt priorities
typedef enum {
    INT_PRIORITY_CRITICAL = 0,      // Critical system interrupts
    INT_PRIORITY_HIGH = 1,          // High priority interrupts
    INT_PRIORITY_NORMAL = 2,        // Normal priority interrupts
    INT_PRIORITY_LOW = 3,           // Low priority interrupts
    INT_PRIORITY_BACKGROUND = 4     // Background interrupts
} interrupt_priority_t;

// Interrupt handler types
typedef enum {
    INT_HANDLER_FAST,               // Fast interrupt handler (atomic context)
    INT_HANDLER_SLOW,               // Slow interrupt handler (may sleep)
    INT_HANDLER_THREADED            // Threaded interrupt handler
} interrupt_handler_type_t;

// Forward declarations
typedef struct interrupt_frame interrupt_frame_t;
typedef struct interrupt_handler interrupt_handler_t;
typedef struct interrupt_controller interrupt_controller_t;

// CPU register state during interrupt
struct interrupt_frame {
    // Pushed by CPU (in reverse order)
    uint64_t ss;            // Stack segment
    uint64_t rsp;           // Stack pointer
    uint64_t rflags;        // Flags register
    uint64_t cs;            // Code segment
    uint64_t rip;           // Instruction pointer
    uint64_t error_code;    // Error code (if applicable)
    
    // Pushed by interrupt stub
    uint64_t rax, rbx, rcx, rdx;    // General purpose registers
    uint64_t rsi, rdi;              // Source/destination index
    uint64_t rbp;                   // Base pointer
    uint64_t r8, r9, r10, r11;      // Extended registers
    uint64_t r12, r13, r14, r15;    // Extended registers
    
    // Segment registers
    uint64_t ds, es, fs, gs;
    
    // Control registers (saved if needed)
    uint64_t cr0, cr2, cr3, cr4;
    
    // Debug registers (saved if needed)
    uint64_t dr0, dr1, dr2, dr3, dr6, dr7;
    
    // Extended state (saved if needed)
    void* fpu_state;        // FPU/SSE/AVX state
} __attribute__((packed));

// Interrupt handler function type
typedef int (*interrupt_handler_func_t)(int vector, interrupt_frame_t* frame, void* data);

// Interrupt handler descriptor
struct interrupt_handler {
    interrupt_handler_func_t handler;   // Handler function
    void* data;                         // Private data
    const char* name;                   // Handler name
    interrupt_handler_type_t type;      // Handler type
    interrupt_priority_t priority;      // Priority level
    
    // Statistics
    struct {
        atomic64_t count;               // Interrupt count
        atomic64_t time_total;          // Total time spent
        atomic64_t time_max;            // Maximum time
        atomic64_t time_min;            // Minimum time
        atomic64_t time_last;           // Last execution time
    } stats;
    
    // Configuration
    struct {
        bool can_share;                 // Can share with other handlers
        bool measure_latency;           // Measure execution latency
        uint32_t cpu_affinity;          // CPU affinity mask
    } config;
    
    // Linkage
    struct interrupt_handler* next;     // Next handler (for shared interrupts)
};

// IDT entry structure (x86-64)
typedef struct idt_entry {
    uint16_t offset_low;        // Offset bits 0-15
    uint16_t selector;          // Code segment selector
    uint8_t ist;                // Interrupt Stack Table offset
    uint8_t type_attr;          // Type and attributes
    uint16_t offset_mid;        // Offset bits 16-31
    uint32_t offset_high;       // Offset bits 32-63
    uint32_t reserved;          // Reserved (must be zero)
} __attribute__((packed)) idt_entry_t;

// IDT descriptor
typedef struct idt_descriptor {
    uint16_t limit;             // IDT limit
    uint64_t base;              // IDT base address
} __attribute__((packed)) idt_descriptor_t;

// APIC registers
#define APIC_ID                     0x020   // Local APIC ID
#define APIC_VERSION                0x030   // Local APIC version
#define APIC_TPR                    0x080   // Task Priority Register
#define APIC_APR                    0x090   // Arbitration Priority Register
#define APIC_PPR                    0x0A0   // Processor Priority Register
#define APIC_EOI                    0x0B0   // End of Interrupt
#define APIC_RRD                    0x0C0   // Remote Read Register
#define APIC_LDR                    0x0D0   // Logical Destination Register
#define APIC_DFR                    0x0E0   // Destination Format Register
#define APIC_SVR                    0x0F0   // Spurious Interrupt Vector Register
#define APIC_ISR_BASE               0x100   // In-Service Register (8 registers)
#define APIC_TMR_BASE               0x180   // Trigger Mode Register (8 registers)
#define APIC_IRR_BASE               0x200   // Interrupt Request Register (8 registers)
#define APIC_ESR                    0x280   // Error Status Register
#define APIC_ICR_LOW                0x300   // Interrupt Command Register (low 32 bits)
#define APIC_ICR_HIGH               0x310   // Interrupt Command Register (high 32 bits)
#define APIC_LVT_TIMER              0x320   // Local Vector Table - Timer
#define APIC_LVT_THERMAL            0x330   // Local Vector Table - Thermal Sensor
#define APIC_LVT_PERF               0x340   // Local Vector Table - Performance Counter
#define APIC_LVT_LINT0              0x350   // Local Vector Table - LINT0
#define APIC_LVT_LINT1              0x360   // Local Vector Table - LINT1
#define APIC_LVT_ERROR              0x370   // Local Vector Table - Error
#define APIC_TIMER_ICR              0x380   // Timer Initial Count Register
#define APIC_TIMER_CCR              0x390   // Timer Current Count Register
#define APIC_TIMER_DCR              0x3E0   // Timer Divide Configuration Register

// Interrupt controller interface
struct interrupt_controller {
    const char* name;           // Controller name
    
    // Operations
    int (*init)(void);          // Initialize controller
    void (*cleanup)(void);      // Cleanup controller
    int (*enable_irq)(int irq); // Enable interrupt
    int (*disable_irq)(int irq);// Disable interrupt
    int (*mask_irq)(int irq);   // Mask interrupt
    int (*unmask_irq)(int irq); // Unmask interrupt
    void (*eoi)(int irq);       // End of interrupt
    int (*set_affinity)(int irq, uint32_t cpu_mask); // Set CPU affinity
    int (*get_irq_priority)(int irq); // Get interrupt priority
    int (*set_irq_priority)(int irq, int priority); // Set interrupt priority
    
    // IPI operations
    void (*send_ipi)(int cpu, int vector); // Send IPI
    void (*send_ipi_all)(int vector);      // Send IPI to all CPUs
    void (*send_ipi_others)(int vector);   // Send IPI to other CPUs
    
    // Statistics
    struct {
        atomic64_t irqs_handled;    // Total IRQs handled
        atomic64_t ipis_sent;       // IPIs sent
        atomic64_t ipis_received;   // IPIs received
        atomic64_t spurious_irqs;   // Spurious interrupts
    } stats;
    
    void* private_data;         // Controller private data
};

// System call frame (for syscall interrupt)
typedef struct syscall_frame {
    uint64_t rax;               // System call number / return value
    uint64_t rdi, rsi, rdx;     // Arguments 1-3
    uint64_t r10, r8, r9;       // Arguments 4-6
    uint64_t rcx;               // Return address (set by syscall instruction)
    uint64_t r11;               // Saved flags (set by syscall instruction)
    
    // Additional context
    uint64_t rbx, rbp;          // Preserved registers
    uint64_t r12, r13, r14, r15; // Preserved registers
    uint64_t rsp;               // User stack pointer
} __attribute__((packed)) syscall_frame_t;

// Interrupt management system
typedef struct interrupt_manager {
    bool initialized;
    
    // IDT management
    idt_entry_t idt[IDT_ENTRIES];          // Interrupt descriptor table
    idt_descriptor_t idt_descriptor;        // IDT descriptor
    
    // Interrupt handlers
    interrupt_handler_t* handlers[IDT_ENTRIES]; // Interrupt handlers
    spinlock_t handler_locks[IDT_ENTRIES];      // Per-vector locks
    
    // Interrupt controller
    interrupt_controller_t* controller;     // Current interrupt controller
    
    // Statistics
    struct {
        atomic64_t total_interrupts;       // Total interrupts handled
        atomic64_t exceptions_handled;     // CPU exceptions handled
        atomic64_t syscalls_handled;       // System calls handled
        atomic64_t spurious_interrupts;    // Spurious interrupts
        atomic64_t nested_interrupts;      // Nested interrupts
        atomic64_t interrupt_storms;       // Interrupt storms detected
        
        // Per-vector statistics
        atomic64_t vector_counts[IDT_ENTRIES];  // Per-vector counts
        atomic64_t vector_time[IDT_ENTRIES];    // Per-vector total time
        
        // Latency statistics
        atomic64_t min_latency;            // Minimum interrupt latency
        atomic64_t max_latency;            // Maximum interrupt latency
        atomic64_t avg_latency;            // Average interrupt latency
        atomic64_t total_latency;          // Total latency
    } stats;
    
    // Configuration
    struct {
        bool measure_latency;              // Measure interrupt latency
        bool detect_storms;                // Detect interrupt storms
        uint32_t storm_threshold;          // Storm detection threshold
        uint32_t storm_window;             // Storm detection window (ms)
        bool log_exceptions;               // Log exception details
        bool profile_interrupts;           // Profile interrupt handlers
    } config;
    
    // Exception handling
    struct {
        bool (*exception_handlers[32])(int vector, interrupt_frame_t* frame);
        const char* exception_names[32];   // Exception names
        atomic64_t exception_counts[32];   // Exception counts
    } exceptions;
    
    // System call interface
    struct {
        bool (*syscall_handler)(syscall_frame_t* frame);
        atomic64_t syscall_count;          // System call count
        atomic64_t syscall_time;           // Total syscall time
        bool syscall_profiling;            // Profile system calls
    } syscalls;
    
    // Debugging and profiling
    struct {
        bool debug_mode;                   // Debug mode enabled
        bool trace_interrupts;             // Trace interrupt execution
        struct list_head trace_buffer;     // Interrupt trace buffer
        spinlock_t trace_lock;             // Trace buffer lock
    } debug;
    
    spinlock_t global_lock;                // Global interrupt manager lock
} interrupt_manager_t;

// Global interrupt manager
extern interrupt_manager_t* int_mgr;

// Core interrupt management API

/**
 * Initialize the interrupt management system
 * @return 0 on success, negative error code on failure
 */
int interrupt_init(void);

/**
 * Late initialization after other subsystems
 * @return 0 on success, negative error code on failure
 */
int interrupt_late_init(void);

/**
 * Cleanup interrupt management system
 */
void interrupt_cleanup(void);

// IDT management

/**
 * Set IDT entry
 * @param vector Interrupt vector
 * @param handler Handler function address
 * @param selector Code segment selector
 * @param type_attr Type and attributes
 * @param ist Interrupt Stack Table offset
 */
void idt_set_entry(int vector, uint64_t handler, uint16_t selector, 
                   uint8_t type_attr, uint8_t ist);

/**
 * Load IDT
 */
void idt_load(void);

/**
 * Get IDT entry
 * @param vector Interrupt vector
 * @return IDT entry or NULL if invalid
 */
idt_entry_t* idt_get_entry(int vector);

// Interrupt handler management

/**
 * Register interrupt handler
 * @param vector Interrupt vector
 * @param handler Handler function
 * @param data Private data
 * @param name Handler name
 * @param type Handler type
 * @param priority Priority level
 * @return 0 on success, negative error code on failure
 */
int interrupt_register_handler(int vector, interrupt_handler_func_t handler,
                              void* data, const char* name,
                              interrupt_handler_type_t type,
                              interrupt_priority_t priority);

/**
 * Unregister interrupt handler
 * @param vector Interrupt vector
 * @param handler Handler function
 * @return 0 on success, negative error code on failure
 */
int interrupt_unregister_handler(int vector, interrupt_handler_func_t handler);

/**
 * Enable interrupt vector
 * @param vector Interrupt vector
 * @return 0 on success, negative error code on failure
 */
int interrupt_enable(int vector);

/**
 * Disable interrupt vector
 * @param vector Interrupt vector
 * @return 0 on success, negative error code on failure
 */
int interrupt_disable(int vector);

/**
 * Mask interrupt (temporarily disable)
 * @param vector Interrupt vector
 * @return 0 on success, negative error code on failure
 */
int interrupt_mask(int vector);

/**
 * Unmask interrupt (re-enable)
 * @param vector Interrupt vector
 * @return 0 on success, negative error code on failure
 */
int interrupt_unmask(int vector);

// Interrupt control

/**
 * Disable interrupts (CLI)
 * @return Previous interrupt flag state
 */
static inline unsigned long interrupt_disable_save(void) {
    unsigned long flags;
    asm volatile("pushfq; popq %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

/**
 * Enable interrupts (STI)
 */
static inline void interrupt_enable_restore(unsigned long flags) {
    asm volatile("pushq %0; popfq" :: "r"(flags) : "memory");
}

/**
 * Check if interrupts are enabled
 * @return True if interrupts are enabled
 */
static inline bool interrupts_enabled(void) {
    unsigned long flags;
    asm volatile("pushfq; popq %0" : "=r"(flags));
    return (flags & 0x200) != 0;  // IF flag
}

// Exception handling

/**
 * Register exception handler
 * @param vector Exception vector (0-31)
 * @param handler Handler function
 * @return 0 on success, negative error code on failure
 */
int exception_register_handler(int vector, 
                              bool (*handler)(int vector, interrupt_frame_t* frame));

/**
 * Unregister exception handler
 * @param vector Exception vector
 * @return 0 on success, negative error code on failure
 */
int exception_unregister_handler(int vector);

// System call interface

/**
 * Register system call handler
 * @param handler System call handler function
 * @return 0 on success, negative error code on failure
 */
int syscall_register_handler(bool (*handler)(syscall_frame_t* frame));

/**
 * Unregister system call handler
 */
void syscall_unregister_handler(void);

// APIC management

/**
 * Initialize Local APIC
 * @return 0 on success, negative error code on failure
 */
int apic_init(void);

/**
 * Send End of Interrupt (EOI)
 * @param vector Interrupt vector
 */
void apic_eoi(int vector);

/**
 * Send IPI to specific CPU
 * @param cpu Target CPU
 * @param vector Interrupt vector
 */
void apic_send_ipi(int cpu, int vector);

/**
 * Send IPI to all CPUs
 * @param vector Interrupt vector
 */
void apic_send_ipi_all(int vector);

/**
 * Send IPI to all other CPUs
 * @param vector Interrupt vector
 */
void apic_send_ipi_others(int vector);

/**
 * Set interrupt affinity
 * @param vector Interrupt vector
 * @param cpu_mask CPU mask
 * @return 0 on success, negative error code on failure
 */
int interrupt_set_affinity(int vector, uint32_t cpu_mask);

// Statistics and debugging

/**
 * Get interrupt statistics
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int interrupt_get_stats(struct interrupt_stats* stats);

/**
 * Get vector statistics
 * @param vector Interrupt vector
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int interrupt_get_vector_stats(int vector, struct vector_stats* stats);

/**
 * Reset interrupt statistics
 */
void interrupt_reset_stats(void);

/**
 * Dump interrupt information
 * @param vector Specific vector (-1 for all)
 */
void interrupt_dump_info(int vector);

/**
 * Enable interrupt tracing
 * @param enable True to enable, false to disable
 */
void interrupt_set_tracing(bool enable);

/**
 * Dump interrupt trace buffer
 */
void interrupt_dump_trace(void);

// Low-level interrupt entry points (implemented in assembly)
extern void interrupt_stub_0(void);   // Divide error
extern void interrupt_stub_1(void);   // Debug
extern void interrupt_stub_2(void);   // NMI
extern void interrupt_stub_3(void);   // Breakpoint
extern void interrupt_stub_4(void);   // Overflow
extern void interrupt_stub_5(void);   // Bound range
extern void interrupt_stub_6(void);   // Invalid opcode
extern void interrupt_stub_7(void);   // Device not available
extern void interrupt_stub_8(void);   // Double fault
extern void interrupt_stub_10(void);  // Invalid TSS
extern void interrupt_stub_11(void);  // Segment not present
extern void interrupt_stub_12(void);  // Stack fault
extern void interrupt_stub_13(void);  // General protection
extern void interrupt_stub_14(void);  // Page fault
extern void interrupt_stub_16(void);  // x87 FPU error
extern void interrupt_stub_17(void);  // Alignment check
extern void interrupt_stub_18(void);  // Machine check
extern void interrupt_stub_19(void);  // SIMD FPU error

// Hardware interrupt stubs
extern void interrupt_stub_32(void);  // Timer
extern void interrupt_stub_33(void);  // Keyboard
extern void interrupt_stub_128(void); // System call

// Common interrupt handler (called from assembly stubs)
void interrupt_common_handler(int vector, interrupt_frame_t* frame);

// Exception handlers
bool handle_divide_error(int vector, interrupt_frame_t* frame);
bool handle_debug_exception(int vector, interrupt_frame_t* frame);
bool handle_page_fault(int vector, interrupt_frame_t* frame);
bool handle_general_protection_fault(int vector, interrupt_frame_t* frame);
bool handle_double_fault(int vector, interrupt_frame_t* frame);

// Utility macros
#define INT_VECTOR_VALID(v) ((v) >= 0 && (v) < IDT_ENTRIES)
#define INT_IS_EXCEPTION(v) ((v) < 32)
#define INT_IS_HARDWARE(v) ((v) >= 32 && (v) < 48)
#define INT_IS_SOFTWARE(v) ((v) >= 48)

#ifdef __cplusplus
}
#endif

#endif // INTERRUPT_PRODUCTION_H
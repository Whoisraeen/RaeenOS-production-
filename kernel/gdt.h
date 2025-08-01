#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <string.h>

#define GDT_ENTRIES 6

// GDT entry structure for code/data segments
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

// GDT entry structure for system segments (TSS)
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  limit_high_gran;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed)) gdt_system_entry_t;

// GDT pointer structure
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

// Task State Segment (TSS) structure for 64-bit
typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed)) tss_t;

// Access byte flags
#define GDT_ACCESS_PRESENT 0x80
#define GDT_ACCESS_USER    0x60
#define GDT_ACCESS_EXEC    0x08
#define GDT_ACCESS_RW      0x02
#define GDT_ACCESS_TSS     0x09

// Granularity byte flags
#define GDT_GRAN_4K        0x80
#define GDT_GRAN_32_BIT    0x40
#define GDT_GRAN_LONG_MODE 0x20

// External assembly functions
extern void gdt_flush(uint64_t);
extern void tss_flush(void);

// GDT initialization function
void gdt_init();

#endif // GDT_H

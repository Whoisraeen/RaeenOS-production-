#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "types.h"

// The magic number for the Multiboot 1 header.
#define MULTIBOOT_BOOTLOADER_MAGIC   0x2BADB002

// Flags for the Multiboot header.
#define MULTIBOOT_PAGE_ALIGN         0x00000001
#define MULTIBOOT_MEMORY_INFO        0x00000002
#define MULTIBOOT_AOUT_KLUDGE        0x00010000

// Multiboot information structure.
typedef struct multiboot_info {
    uint32_t flags; // Multiboot info version and flags

    // Available if bit 0 is set
    uint32_t mem_lower; // Amount of lower memory (in KBytes)
    uint32_t mem_upper; // Amount of upper memory (in KBytes)

    // Available if bit 1 is set
    uint32_t boot_device; // Boot device
    uint32_t cmdline;     // Kernel command line

    // Available if bit 2 is set
    uint32_t mods_count;  // Number of modules
    uint32_t mods_addr;   // Address of module structure array

    // Available if bit 3 is set
    uint32_t syms[3];     // Symbol table info (a.out or ELF)

    // Available if bit 4 is set
    uint32_t mmap_length; // Length of memory map buffer
    uint32_t mmap_addr;   // Address of memory map buffer

    // Available if bit 5 is set
    uint32_t drives_length; // Length of drive buffer
    uint32_t drives_addr;   // Address of drive buffer

    // Available if bit 6 is set
    uint32_t config_table;  // ROM configuration table

    // Available if bit 7 is set
    uint32_t boot_loader_name; // Boot loader name

    // Available if bit 8 is set
    uint32_t apm_table;     // APM table

    // Available if bit 9 is set
    uint32_t vbe_control_info; // VBE control information
    uint32_t vbe_mode_info;    // VBE mode information
    uint16_t vbe_mode;         // VBE mode
    uint16_t vbe_interface_seg; // VBE interface segment
    uint16_t vbe_interface_off; // VBE interface offset
    uint16_t vbe_interface_len; // VBE interface length

    // Available if bit 10 is set
    uint64_t framebuffer_addr;   // Framebuffer physical address
    uint32_t framebuffer_pitch;  // Pitch of the framebuffer
    uint32_t framebuffer_width;  // Width of the framebuffer
    uint32_t framebuffer_height; // Height of the framebuffer
    uint8_t  framebuffer_bpp;    // Bits per pixel
    uint8_t  framebuffer_type;   // Type of framebuffer
    uint8_t  color_info[6];      // Color information

} __attribute__((packed)) multiboot_info_t;

// Memory map entry structure
typedef struct multiboot_mmap_entry {
    uint32_t size;     // Size of the entry
    uint64_t addr;     // Start address of the memory region
    uint64_t len;      // Length of the memory region
    uint32_t type;     // Type of the memory region
} __attribute__((packed)) multiboot_mmap_entry_t;

// Memory region types
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5

#endif // MULTIBOOT_H

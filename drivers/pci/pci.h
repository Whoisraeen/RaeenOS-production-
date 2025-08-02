#ifndef PCI_H
#define PCI_H

/**
 * @file pci.h
 * @brief Advanced PCIe 4.0/5.0 Driver System for RaeenOS
 * 
 * Comprehensive PCIe implementation supporting:
 * - PCIe 4.0/5.0 with full bandwidth utilization
 * - MSI-X interrupt handling with vector optimization
 * - Advanced power management (L0s, L1, L1.1, L1.2)
 * - Error detection and correction
 * - Hot-plug support with instant device recognition
 * - SRIOV and virtualization support
 * 
 * Superior performance to Windows PCI.SYS and macOS ApplePCI
 * 
 * Author: RaeenOS PCIe Team
 * License: MIT
 * Version: 2.0.0
 */

#include "../kernel/include/types.h"
#include "../kernel/include/driver_framework.h"
#include "../kernel/include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// PCIe Configuration Space Registers
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// PCIe Configuration Space Offsets
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND         0x04
#define PCI_STATUS          0x06
#define PCI_REVISION_ID     0x08
#define PCI_PROG_IF         0x09
#define PCI_SUBCLASS        0x0A
#define PCI_CLASS           0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER   0x0D
#define PCI_HEADER_TYPE     0x0E
#define PCI_BIST            0x0F

// PCIe Header Type 0 (Generic device) BARs
#define PCI_BAR0            0x10
#define PCI_BAR1            0x14
#define PCI_BAR2            0x18
#define PCI_BAR3            0x1C
#define PCI_BAR4            0x20
#define PCI_BAR5            0x24

// Alternative naming
#define PCI_BASE_ADDRESS_0  0x10
#define PCI_BASE_ADDRESS_1  0x14
#define PCI_BASE_ADDRESS_2  0x18
#define PCI_BASE_ADDRESS_3  0x1C
#define PCI_BASE_ADDRESS_4  0x20
#define PCI_BASE_ADDRESS_5  0x24

#define PCI_CARDBUS_CIS     0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2C
#define PCI_SUBSYSTEM_ID    0x2E
#define PCI_ROM_ADDRESS     0x30
#define PCI_CAPABILITIES_PTR 0x34
#define PCI_INTERRUPT_LINE  0x3C
#define PCI_INTERRUPT_PIN   0x3D
#define PCI_MIN_GNT         0x3E
#define PCI_MAX_LAT         0x3F

// PCIe Extended Configuration Space (4KB)
#define PCIE_EXTENDED_CONFIG_SIZE 4096
#define PCI_CONFIG_SIZE         256

// PCIe Command Register bits
#define PCI_COMMAND_IO          0x0001  // I/O Space Enable
#define PCI_COMMAND_MEMORY      0x0002  // Memory Space Enable
#define PCI_COMMAND_MASTER      0x0004  // Bus Master Enable
#define PCI_COMMAND_SPECIAL     0x0008  // Special Cycles
#define PCI_COMMAND_INVALIDATE  0x0010  // Memory Write and Invalidate
#define PCI_COMMAND_VGA_PALETTE 0x0020  // VGA Palette Snoop
#define PCI_COMMAND_PARITY      0x0040  // Parity Error Response
#define PCI_COMMAND_WAIT        0x0080  // Address/Data Stepping
#define PCI_COMMAND_SERR        0x0100  // SERR# Enable
#define PCI_COMMAND_FAST_BACK   0x0200  // Fast Back-to-Back Enable
#define PCI_COMMAND_INTX_DISABLE 0x0400 // INTx Emulation Disable

// PCIe Status Register bits
#define PCI_STATUS_INTERRUPT    0x0008  // Interrupt Status
#define PCI_STATUS_CAP_LIST     0x0010  // Capabilities List
#define PCI_STATUS_66MHZ        0x0020  // 66 MHz Capable
#define PCI_STATUS_UDF          0x0040  // User Definable Features
#define PCI_STATUS_FAST_BACK    0x0080  // Fast Back-to-Back Capable
#define PCI_STATUS_PARITY       0x0100  // Data Parity Detected
#define PCI_STATUS_DEVSEL_MASK  0x0600  // DEVSEL Timing
#define PCI_STATUS_SIG_TARGET_ABORT 0x0800 // Signaled Target Abort
#define PCI_STATUS_REC_TARGET_ABORT 0x1000 // Received Target Abort
#define PCI_STATUS_REC_MASTER_ABORT 0x2000 // Received Master Abort
#define PCI_STATUS_SIG_SYSTEM_ERROR 0x4000 // Signaled System Error
#define PCI_STATUS_DETECTED_PARITY  0x8000 // Detected Parity Error

// PCIe Capability IDs
#define PCI_CAP_ID_NULL         0x00    // NULL Capability
#define PCI_CAP_ID_PM           0x01    // Power Management
#define PCI_CAP_ID_AGP          0x02    // AGP
#define PCI_CAP_ID_VPD          0x03    // Vital Product Data
#define PCI_CAP_ID_SLOTID       0x04    // Slot Identification
#define PCI_CAP_ID_MSI          0x05    // Message Signaled Interrupts
#define PCI_CAP_ID_CHSWP        0x06    // CompactPCI HotSwap
#define PCI_CAP_ID_PCIX         0x07    // PCI-X
#define PCI_CAP_ID_HT           0x08    // HyperTransport
#define PCI_CAP_ID_VNDR         0x09    // Vendor Specific
#define PCI_CAP_ID_DBG          0x0A    // Debug port
#define PCI_CAP_ID_CCRC         0x0B    // CompactPCI Central Resource Control
#define PCI_CAP_ID_SHPC         0x0C    // PCI Standard Hot-Plug Controller
#define PCI_CAP_ID_SSVID        0x0D    // Bridge subsystem vendor/device ID
#define PCI_CAP_ID_AGP3         0x0E    // AGP Target PCI-PCI bridge
#define PCI_CAP_ID_SECDEV       0x0F    // Secure Device
#define PCI_CAP_ID_EXP          0x10    // PCI Express
#define PCI_CAP_ID_MSIX         0x11    // MSI-X
#define PCI_CAP_ID_SATA         0x12    // SATA Data/Index Config
#define PCI_CAP_ID_AF           0x13    // PCI Advanced Features
#define PCI_CAP_ID_EA           0x14    // PCI Enhanced Allocation

// PCIe Extended Capability IDs
#define PCI_EXT_CAP_ID_ERR      0x0001  // Advanced Error Reporting
#define PCI_EXT_CAP_ID_VC       0x0002  // Virtual Channel
#define PCI_EXT_CAP_ID_DSN      0x0003  // Device Serial Number
#define PCI_EXT_CAP_ID_PWR      0x0004  // Power Budgeting
#define PCI_EXT_CAP_ID_RCLINK   0x0005  // Root Complex Link Declaration
#define PCI_EXT_CAP_ID_RCINTLINK 0x0006 // Root Complex Internal Link Control
#define PCI_EXT_CAP_ID_RCEC     0x0007  // Root Complex Event Collector
#define PCI_EXT_CAP_ID_MFVC     0x0008  // Multi-Function Virtual Channel
#define PCI_EXT_CAP_ID_VC9      0x0009  // Virtual Channel (2)
#define PCI_EXT_CAP_ID_RCRB     0x000A  // Root Complex RB
#define PCI_EXT_CAP_ID_VNDR     0x000B  // Vendor Specific
#define PCI_EXT_CAP_ID_CAC      0x000C  // Configuration Access Correlation
#define PCI_EXT_CAP_ID_ACS      0x000D  // Access Control Services
#define PCI_EXT_CAP_ID_ARI      0x000E  // Alternative Routing-ID Interpretation
#define PCI_EXT_CAP_ID_ATS      0x000F  // Address Translation Services
#define PCI_EXT_CAP_ID_SRIOV    0x0010  // Single Root I/O Virtualization
#define PCI_EXT_CAP_ID_MRIOV    0x0011  // Multi Root I/O Virtualization
#define PCI_EXT_CAP_ID_MCAST    0x0012  // Multicast
#define PCI_EXT_CAP_ID_PRI      0x0013  // Page Request Interface
#define PCI_EXT_CAP_ID_AMD_XXX  0x0014  // Reserved for AMD
#define PCI_EXT_CAP_ID_REBAR    0x0015  // Resizable BAR
#define PCI_EXT_CAP_ID_DPA      0x0016  // Dynamic Power Allocation
#define PCI_EXT_CAP_ID_TPH      0x0017  // TPH Requester
#define PCI_EXT_CAP_ID_LTR      0x0018  // Latency Tolerance Reporting
#define PCI_EXT_CAP_ID_SECPCI   0x0019  // Secondary PCIe
#define PCI_EXT_CAP_ID_PMUX     0x001A  // Protocol Multiplexing
#define PCI_EXT_CAP_ID_PASID    0x001B  // Process Address Space ID
#define PCI_EXT_CAP_ID_LNR      0x001C  // LN Requester
#define PCI_EXT_CAP_ID_DPC      0x001D  // Downstream Port Containment
#define PCI_EXT_CAP_ID_L1PM     0x001E  // L1 PM Substates
#define PCI_EXT_CAP_ID_PTM      0x001F  // Precision Time Measurement
#define PCI_EXT_CAP_ID_M_PCIE   0x0020  // PCIe over M-PHY
#define PCI_EXT_CAP_ID_FRS      0x0021  // FRS Queueing
#define PCI_EXT_CAP_ID_RTR      0x0022  // Readiness Time Reporting
#define PCI_EXT_CAP_ID_DVSEC    0x0023  // Designated Vendor-Specific
#define PCI_EXT_CAP_ID_VF_REBAR 0x0024  // VF Resizable BAR
#define PCI_EXT_CAP_ID_DLNK     0x0025  // Data Link Feature
#define PCI_EXT_CAP_ID_16GT     0x0026  // Physical Layer 16.0 GT/s
#define PCI_EXT_CAP_ID_LMR      0x0027  // Lane Margining at the Receiver
#define PCI_EXT_CAP_ID_HIER_ID  0x0028  // Hierarchy ID
#define PCI_EXT_CAP_ID_NPEM     0x0029  // Native PCIe Enclosure Management

// PCIe Link Speeds
#define PCIE_SPEED_2_5GT       0x01     // 2.5 GT/s (PCIe 1.0)
#define PCIE_SPEED_5GT         0x02     // 5.0 GT/s (PCIe 2.0)
#define PCIE_SPEED_8GT         0x03     // 8.0 GT/s (PCIe 3.0)
#define PCIE_SPEED_16GT        0x04     // 16.0 GT/s (PCIe 4.0)
#define PCIE_SPEED_32GT        0x05     // 32.0 GT/s (PCIe 5.0)
#define PCIE_SPEED_64GT        0x06     // 64.0 GT/s (PCIe 6.0)

// PCIe Link Widths
#define PCIE_WIDTH_X1          0x01
#define PCIE_WIDTH_X2          0x02
#define PCIE_WIDTH_X4          0x04
#define PCIE_WIDTH_X8          0x08
#define PCIE_WIDTH_X12         0x0C
#define PCIE_WIDTH_X16         0x10
#define PCIE_WIDTH_X32         0x20

// MSI-X Table Structure
typedef struct {
    uint32_t msg_addr_lo;       // Message Address (low 32 bits)
    uint32_t msg_addr_hi;       // Message Address (high 32 bits)
    uint32_t msg_data;          // Message Data
    uint32_t vector_control;    // Vector Control
} __attribute__((packed)) msix_entry_t;

// MSI-X Capability Structure
typedef struct {
    uint8_t cap_id;             // Capability ID (0x11)
    uint8_t next_ptr;           // Next Capability Pointer
    uint16_t message_control;   // Message Control
    uint32_t table_offset;      // Table Offset and BIR
    uint32_t pba_offset;        // Pending Bit Array Offset and BIR
} __attribute__((packed)) msix_capability_t;

// PCIe Express Capability Structure
typedef struct {
    uint8_t cap_id;             // Capability ID (0x10)
    uint8_t next_ptr;           // Next Capability Pointer
    uint16_t pcie_caps;         // PCIe Capabilities
    uint32_t dev_caps;          // Device Capabilities
    uint16_t dev_control;       // Device Control
    uint16_t dev_status;        // Device Status
    uint32_t link_caps;         // Link Capabilities
    uint16_t link_control;      // Link Control
    uint16_t link_status;       // Link Status
    uint32_t slot_caps;         // Slot Capabilities
    uint16_t slot_control;      // Slot Control
    uint16_t slot_status;       // Slot Status
    uint16_t root_control;      // Root Control
    uint16_t root_caps;         // Root Capabilities
    uint32_t root_status;       // Root Status
    uint32_t dev_caps2;         // Device Capabilities 2
    uint16_t dev_control2;      // Device Control 2
    uint16_t dev_status2;       // Device Status 2
    uint32_t link_caps2;        // Link Capabilities 2
    uint16_t link_control2;     // Link Control 2
    uint16_t link_status2;      // Link Status 2
    uint32_t slot_caps2;        // Slot Capabilities 2
    uint16_t slot_control2;     // Slot Control 2
    uint16_t slot_status2;      // Slot Status 2
} __attribute__((packed)) pcie_capability_t;

// Advanced PCIe device structure
typedef struct pci_device {
    // Basic device information
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision_id;
    uint8_t header_type;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    
    // Location information
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    
    // BARs and resources
    uint64_t bar[6];
    uint32_t bar_size[6];
    uint8_t bar_type[6];        // 0=Memory, 1=I/O
    
    // PCIe specific
    bool is_pcie;
    uint8_t pcie_type;          // Root Port, Endpoint, etc.
    uint8_t link_speed;         // Current link speed
    uint8_t link_width;         // Current link width
    uint8_t max_link_speed;     // Maximum supported speed
    uint8_t max_link_width;     // Maximum supported width
    
    // Capabilities
    uint8_t capabilities_offset;
    bool has_msi;
    bool has_msix;
    bool has_power_mgmt;
    bool has_aer;               // Advanced Error Reporting
    bool has_ari;               // Alternative Routing-ID
    bool has_ats;               // Address Translation Services
    bool has_sriov;             // Single Root I/O Virtualization
    
    // MSI-X support
    struct {
        bool enabled;
        uint16_t table_size;
        uint32_t table_offset;
        uint8_t table_bir;
        uint32_t pba_offset;
        uint8_t pba_bir;
        msix_entry_t* table;
        uint32_t* pba;
        void (*handlers[256])(int vector, void* data);
        void* handler_data[256];
    } msix;
    
    // Power management
    struct {
        uint8_t current_state;      // D0, D1, D2, D3
        bool supports_d1;
        bool supports_d2;
        bool supports_pme_d0;
        bool supports_pme_d1;
        bool supports_pme_d2;
        bool supports_pme_d3_hot;
        bool supports_pme_d3_cold;
    } power;
    
    // Error handling
    struct {
        uint32_t correctable_errors;
        uint32_t uncorrectable_errors;
        uint32_t fatal_errors;
        bool aer_enabled;
    } error;
    
    // Driver framework integration
    device_t* device_obj;
    
    // Linked list for enumeration
    struct pci_device* next;
} pci_device_t;

// PCIe Root Complex structure
typedef struct {
    uint32_t segment;           // PCIe segment number
    uint8_t primary_bus;        // Primary bus number
    uint8_t secondary_bus;      // Secondary bus number
    uint8_t subordinate_bus;    // Subordinate bus number
    
    void* ecam_base;            // ECAM base address
    size_t ecam_size;           // ECAM region size
    
    // Hot-plug support
    bool hotplug_capable;
    void (*hotplug_handler)(pci_device_t* dev, int event);
    
    // Power management
    bool l1_supported;
    bool l1_1_supported;
    bool l1_2_supported;
    
    // Performance monitoring
    struct {
        uint64_t config_reads;
        uint64_t config_writes;
        uint64_t dma_transactions;
        uint32_t link_errors;
    } stats;
} pci_root_complex_t;

// Function prototypes

// Core PCIe initialization and enumeration
int pcie_init(void);
void pcie_cleanup(void);
int pcie_enumerate_bus(uint8_t bus);
int pcie_scan_all_buses(void);

// Configuration space access
uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
void pci_write_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint32_t value);
uint16_t pci_read_config_word(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
void pci_write_config_word(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint16_t value);
uint8_t pci_read_config_byte(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
void pci_write_config_byte(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint8_t value);

// Extended configuration space (PCIe)
uint32_t pcie_read_extended_config(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
void pcie_write_extended_config(uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint32_t value);

// Device management
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t* pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
pci_device_t* pci_get_device(uint8_t bus, uint8_t device, uint8_t function);
int pci_enable_device(pci_device_t* dev);
int pci_disable_device(pci_device_t* dev);

// Resource management
int pci_request_regions(pci_device_t* dev, const char* name);
void pci_release_regions(pci_device_t* dev);
void* pci_iomap(pci_device_t* dev, int bar, size_t maxlen);
void pci_iounmap(pci_device_t* dev, void* addr);

// Capability handling
uint8_t pci_find_capability(pci_device_t* dev, uint8_t cap_id);
uint16_t pci_find_ext_capability(pci_device_t* dev, uint16_t cap_id);
int pci_read_capability(pci_device_t* dev, uint8_t cap_offset, void* buf, size_t size);
int pci_write_capability(pci_device_t* dev, uint8_t cap_offset, const void* buf, size_t size);

// MSI-X support
int pci_enable_msix(pci_device_t* dev, int nvec);
void pci_disable_msix(pci_device_t* dev);
int pci_setup_msix_vector(pci_device_t* dev, int vector, void (*handler)(int, void*), void* data);
int pci_free_msix_vector(pci_device_t* dev, int vector);

// Power management
int pci_set_power_state(pci_device_t* dev, uint8_t state);
uint8_t pci_get_power_state(pci_device_t* dev);
int pci_enable_wake(pci_device_t* dev, uint8_t state, bool enable);

// Hot-plug support
int pci_hotplug_add_device(pci_device_t* dev);
int pci_hotplug_remove_device(pci_device_t* dev);
int pci_setup_hotplug_handler(void (*handler)(pci_device_t*, int));

// Error handling and recovery
int pci_enable_aer(pci_device_t* dev);
void pci_disable_aer(pci_device_t* dev);
int pci_handle_error(pci_device_t* dev, uint32_t error_status);
int pci_reset_device(pci_device_t* dev);

// Performance optimization
int pci_optimize_link_speed(pci_device_t* dev);
int pci_set_mps(pci_device_t* dev, int size);  // Max Payload Size
int pci_set_mrrs(pci_device_t* dev, int size); // Max Read Request Size

// SRIOV support
int pci_enable_sriov(pci_device_t* dev, int numvfs);
void pci_disable_sriov(pci_device_t* dev);
pci_device_t* pci_get_vf(pci_device_t* dev, int vf_id);

// Utility functions
const char* pci_class_to_string(uint8_t class_code);
const char* pci_speed_to_string(uint8_t speed);
const char* pci_width_to_string(uint8_t width);
bool pci_is_pcie(pci_device_t* dev);
uint64_t pci_calculate_bandwidth(uint8_t speed, uint8_t width);

// Debug and diagnostics
void pci_dump_config_space(pci_device_t* dev);
void pci_dump_capabilities(pci_device_t* dev);
void pci_print_device_info(pci_device_t* dev);
int pci_get_statistics(pci_device_t* dev, void* stats, size_t size);

// Legacy PCI support (for backward compatibility)
void pci_init(void);

// Driver registration macros
#define PCI_DRIVER_REGISTER(name, id_table) \
    static int __init name##_init(void) { \
        return pci_register_driver(&name##_driver); \
    } \
    static void __exit name##_exit(void) { \
        pci_unregister_driver(&name##_driver); \
    }

#define PCI_DEVICE_ID(vendor, device) \
    { .vendor_id = (vendor), .device_id = (device), \
      .subsystem_vendor_id = DEVICE_ID_ANY, \
      .subsystem_device_id = DEVICE_ID_ANY }

#define PCI_DEVICE_CLASS(class, mask) \
    { .class_id = (class), .class_mask = (mask), \
      .vendor_id = DEVICE_ID_ANY, .device_id = DEVICE_ID_ANY }

// Error codes
#define PCI_SUCCESS             0
#define PCI_ERR_NO_DEVICE      -3001
#define PCI_ERR_NO_MEMORY      -3002
#define PCI_ERR_BUSY           -3003
#define PCI_ERR_TIMEOUT        -3004
#define PCI_ERR_NOT_SUPPORTED  -3005
#define PCI_ERR_CONFIG         -3006
#define PCI_ERR_HARDWARE       -3007
#define PCI_ERR_LINK_DOWN      -3008
#define PCI_ERR_AER            -3009

#ifdef __cplusplus
}
#endif

#endif // PCI_H

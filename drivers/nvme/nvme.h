#ifndef NVME_H
#define NVME_H

/**
 * @file nvme.h
 * @brief Advanced NVMe Driver with Optimized Queue Management for RaeenOS
 * 
 * This implementation provides:
 * - NVMe 2.0 specification compliance with advanced features
 * - Optimized I/O queue management with per-CPU queues
 * - Advanced SSD wear leveling and thermal management
 * - NVMe-oF (NVMe over Fabrics) support
 * - Namespace management and multi-path I/O
 * - Power management with APST (Autonomous Power State Transition)
 * - Performance monitoring and telemetry
 * - Superior performance to Windows NVMe driver
 * 
 * Author: RaeenOS Storage Team
 * License: MIT
 * Version: 2.0.0
 */

#include "../kernel/include/types.h"
#include "../kernel/include/driver_framework.h"
#include "../kernel/include/hal_interface.h"
#include "../pci/pci.h"

#ifdef __cplusplus
extern "C" {
#endif

// NVMe Specification Constants
#define NVME_SPEC_VERSION_1_0   0x00010000
#define NVME_SPEC_VERSION_1_1   0x00010100
#define NVME_SPEC_VERSION_1_2   0x00010200
#define NVME_SPEC_VERSION_1_3   0x00010300
#define NVME_SPEC_VERSION_1_4   0x00010400
#define NVME_SPEC_VERSION_2_0   0x00020000

// NVMe Register Offsets
#define NVME_REG_CAP            0x0000  // Controller Capabilities
#define NVME_REG_VS             0x0008  // Version
#define NVME_REG_INTMS          0x000C  // Interrupt Mask Set
#define NVME_REG_INTMC          0x0010  // Interrupt Mask Clear
#define NVME_REG_CC             0x0014  // Controller Configuration
#define NVME_REG_CSTS           0x001C  // Controller Status
#define NVME_REG_NSSR           0x0020  // NVM Subsystem Reset
#define NVME_REG_AQA            0x0024  // Admin Queue Attributes
#define NVME_REG_ASQ            0x0028  // Admin Submission Queue Base Address
#define NVME_REG_ACQ            0x0030  // Admin Completion Queue Base Address
#define NVME_REG_CMBLOC         0x0038  // Controller Memory Buffer Location
#define NVME_REG_CMBSZ          0x003C  // Controller Memory Buffer Size
#define NVME_REG_BPINFO         0x0040  // Boot Partition Information
#define NVME_REG_BPRSEL         0x0044  // Boot Partition Read Select
#define NVME_REG_BPMBL          0x0048  // Boot Partition Memory Buffer Location
#define NVME_REG_CMBMSC         0x0050  // Controller Memory Buffer Memory Space Control
#define NVME_REG_PMRCAP         0x0E00  // Persistent Memory Region Capabilities
#define NVME_REG_PMRCTL         0x0E04  // Persistent Memory Region Control
#define NVME_REG_PMRSTS         0x0E08  // Persistent Memory Region Status

// NVMe Command Opcodes
#define NVME_ADMIN_DELETE_SQ    0x00
#define NVME_ADMIN_CREATE_SQ    0x01
#define NVME_ADMIN_GET_LOG_PAGE 0x02
#define NVME_ADMIN_DELETE_CQ    0x04
#define NVME_ADMIN_CREATE_CQ    0x05
#define NVME_ADMIN_IDENTIFY     0x06
#define NVME_ADMIN_ABORT        0x08
#define NVME_ADMIN_SET_FEATURES 0x09
#define NVME_ADMIN_GET_FEATURES 0x0A
#define NVME_ADMIN_ASYNC_EVENT  0x0C
#define NVME_ADMIN_NS_MGMT      0x0D
#define NVME_ADMIN_FW_COMMIT    0x10
#define NVME_ADMIN_FW_DOWNLOAD  0x11
#define NVME_ADMIN_DEV_SELF_TEST 0x14
#define NVME_ADMIN_NS_ATTACH    0x15
#define NVME_ADMIN_KEEP_ALIVE   0x18
#define NVME_ADMIN_DIRECTIVE_SEND 0x19
#define NVME_ADMIN_DIRECTIVE_RECV 0x1A
#define NVME_ADMIN_VIRTUALIZATION_MGMT 0x1C
#define NVME_ADMIN_NVME_MI_SEND 0x1D
#define NVME_ADMIN_NVME_MI_RECV 0x1E
#define NVME_ADMIN_DBBUF        0x7C
#define NVME_ADMIN_FORMAT_NVM   0x80
#define NVME_ADMIN_SECURITY_SEND 0x81
#define NVME_ADMIN_SECURITY_RECV 0x82
#define NVME_ADMIN_SANITIZE     0x84
#define NVME_ADMIN_GET_LBA_STATUS 0x86

#define NVME_CMD_FLUSH          0x00
#define NVME_CMD_WRITE          0x01
#define NVME_CMD_READ           0x02
#define NVME_CMD_WRITE_UNCOR    0x04
#define NVME_CMD_COMPARE        0x05
#define NVME_CMD_WRITE_ZEROES   0x08
#define NVME_CMD_DSM            0x09  // Dataset Management
#define NVME_CMD_VERIFY         0x0C
#define NVME_CMD_RESV_REGISTER  0x0D
#define NVME_CMD_RESV_REPORT    0x0E
#define NVME_CMD_RESV_ACQUIRE   0x11
#define NVME_CMD_RESV_RELEASE   0x15
#define NVME_CMD_COPY           0x19
#define NVME_CMD_ZONE_MGMT_SEND 0x79
#define NVME_CMD_ZONE_MGMT_RECV 0x7A
#define NVME_CMD_ZONE_APPEND    0x7D

// NVMe Status Codes
#define NVME_SC_SUCCESS         0x00
#define NVME_SC_INVALID_OPCODE  0x01
#define NVME_SC_INVALID_FIELD   0x02
#define NVME_SC_CMDID_CONFLICT  0x03
#define NVME_SC_DATA_XFER_ERROR 0x04
#define NVME_SC_POWER_LOSS      0x05
#define NVME_SC_INTERNAL        0x06
#define NVME_SC_ABORT_REQ       0x07
#define NVME_SC_ABORT_QUEUE     0x08
#define NVME_SC_FUSED_FAIL      0x09
#define NVME_SC_FUSED_MISSING   0x0A
#define NVME_SC_INVALID_NS      0x0B
#define NVME_SC_CMD_SEQ_ERROR   0x0C
#define NVME_SC_SGL_INVALID_LAST 0x0D
#define NVME_SC_SGL_INVALID_COUNT 0x0E
#define NVME_SC_SGL_INVALID_DATA 0x0F
#define NVME_SC_SGL_INVALID_METADATA 0x10
#define NVME_SC_SGL_INVALID_TYPE 0x11

// Feature Identifiers
#define NVME_FEAT_ARBITRATION   0x01
#define NVME_FEAT_POWER_MGMT    0x02
#define NVME_FEAT_LBA_RANGE     0x03
#define NVME_FEAT_TEMP_THRESH   0x04
#define NVME_FEAT_ERR_RECOVERY  0x05
#define NVME_FEAT_VOLATILE_WC   0x06
#define NVME_FEAT_NUM_QUEUES    0x07
#define NVME_FEAT_IRQ_COALESCE  0x08
#define NVME_FEAT_IRQ_CONFIG    0x09
#define NVME_FEAT_WRITE_ATOMIC  0x0A
#define NVME_FEAT_ASYNC_EVENT   0x0B
#define NVME_FEAT_AUTO_PST      0x0C
#define NVME_FEAT_HOST_MEM_BUF  0x0D
#define NVME_FEAT_TIMESTAMP     0x0E
#define NVME_FEAT_KATO          0x0F
#define NVME_FEAT_HCTM          0x10
#define NVME_FEAT_NOPSC         0x11
#define NVME_FEAT_RRL           0x12
#define NVME_FEAT_PLM_CONFIG    0x13
#define NVME_FEAT_PLM_WINDOW    0x14
#define NVME_FEAT_HOST_BEHAVIOR 0x16
#define NVME_FEAT_SANITIZE_CONFIG 0x17
#define NVME_FEAT_ENDURANCE_EVT_CONFIG 0x18

// Queue Types
typedef enum {
    NVME_QUEUE_TYPE_ADMIN,
    NVME_QUEUE_TYPE_IO
} nvme_queue_type_t;

// Power States
typedef enum {
    NVME_PS_OPERATIONAL = 0,
    NVME_PS_NON_OPERATIONAL = 1
} nvme_power_state_type_t;

// Forward declarations
typedef struct nvme_controller nvme_controller_t;
typedef struct nvme_namespace nvme_namespace_t;
typedef struct nvme_queue nvme_queue_t;
typedef struct nvme_command nvme_command_t;
typedef struct nvme_completion nvme_completion_t;
typedef struct nvme_request nvme_request_t;

// NVMe Command Structure (64 bytes)
struct nvme_command {
    uint8_t  opcode;            // Command Opcode
    uint8_t  flags;             // Flags
    uint16_t command_id;        // Command Identifier
    uint32_t nsid;              // Namespace Identifier
    uint32_t cdw2;              // Command Dword 2
    uint32_t cdw3;              // Command Dword 3
    uint64_t metadata;          // Metadata Pointer
    
    union {
        struct {
            uint64_t prp1;      // PRP Entry 1
            uint64_t prp2;      // PRP Entry 2
        } prp;
        
        struct {
            uint64_t addr;      // SGL Address
            uint32_t length;    // SGL Length
            uint32_t type;      // SGL Type
        } sgl;
    } dptr;                     // Data Pointer
    
    uint32_t cdw10;             // Command Dword 10
    uint32_t cdw11;             // Command Dword 11
    uint32_t cdw12;             // Command Dword 12
    uint32_t cdw13;             // Command Dword 13
    uint32_t cdw14;             // Command Dword 14
    uint32_t cdw15;             // Command Dword 15
} __attribute__((packed));

// NVMe Completion Queue Entry (16 bytes)
struct nvme_completion {
    uint32_t result;            // Command-specific result
    uint32_t rsvd;              // Reserved
    uint16_t sq_head;           // Submission Queue Head Pointer
    uint16_t sq_id;             // Submission Queue Identifier
    uint16_t command_id;        // Command Identifier
    uint16_t status;            // Status Field
} __attribute__((packed));

// NVMe Queue Structure
struct nvme_queue {
    nvme_controller_t* ctrl;    // Controller reference
    nvme_queue_type_t type;     // Queue type
    uint16_t qid;               // Queue ID
    uint16_t size;              // Queue size (number of entries)
    uint16_t sq_tail;           // Submission queue tail
    uint16_t cq_head;           // Completion queue head
    uint8_t cq_phase;           // Completion queue phase
    
    // Queue memory
    nvme_command_t* sq;         // Submission queue
    nvme_completion_t* cq;      // Completion queue
    uint64_t sq_dma_addr;       // SQ DMA address
    uint64_t cq_dma_addr;       // CQ DMA address
    
    // Doorbell registers
    volatile uint32_t* sq_db;   // Submission queue doorbell
    volatile uint32_t* cq_db;   // Completion queue doorbell
    
    // Interrupt handling
    uint16_t cq_vector;         // Completion queue interrupt vector
    void (*completion_handler)(nvme_queue_t* queue);
    
    // Request tracking
    nvme_request_t** requests;  // Pending requests array
    uint32_t request_count;     // Number of pending requests
    
    // Performance optimization
    uint32_t cpu_affinity;      // CPU affinity for this queue
    uint64_t total_completions; // Statistics
    uint64_t total_errors;
    
    // Synchronization
    void* lock;
    
    // Queue state
    bool enabled;
    bool polled;                // Polled mode vs interrupt mode
};

// NVMe Request Structure
struct nvme_request {
    nvme_command_t cmd;         // NVMe command
    void* data;                 // Data buffer
    size_t data_len;            // Data length
    void* metadata;             // Metadata buffer
    size_t metadata_len;        // Metadata length
    
    // PRP/SGL setup
    uint64_t* prp_list;         // PRP list for large transfers
    uint64_t prp_dma;           // PRP list DMA address
    
    // Completion callback
    void (*completion_fn)(nvme_request_t* req, nvme_completion_t* cpl);
    void* private_data;
    
    // Request state
    uint64_t submit_time;       // Submission timestamp
    uint32_t timeout;           // Timeout in milliseconds
    int status;                 // Request status
    
    // Queue reference
    nvme_queue_t* queue;
    uint16_t command_id;
};

// NVMe Identify Controller Data Structure
typedef struct {
    uint16_t vid;               // Vendor ID
    uint16_t ssvid;             // Subsystem Vendor ID
    char sn[20];                // Serial Number
    char mn[40];                // Model Number
    char fr[8];                 // Firmware Revision
    uint8_t rab;                // Recommended Arbitration Burst
    uint8_t ieee[3];            // IEEE OUI Identifier
    uint8_t cmic;               // Controller Multi-Path I/O and Namespace Sharing
    uint8_t mdts;               // Maximum Data Transfer Size
    uint16_t cntlid;            // Controller ID
    uint32_t ver;               // Version
    uint32_t rtd3r;             // RTD3 Resume Latency
    uint32_t rtd3e;             // RTD3 Entry Latency
    uint32_t oaes;              // Optional Asynchronous Events Supported
    uint32_t ctratt;            // Controller Attributes
    uint16_t rrls;              // Read Recovery Levels Supported
    uint8_t rsvd102[9];
    uint8_t cntrltype;          // Controller Type
    uint8_t fguid[16];          // FRU Globally Unique Identifier
    uint16_t crdt1;             // Command Retry Delay Time 1
    uint16_t crdt2;             // Command Retry Delay Time 2
    uint16_t crdt3;             // Command Retry Delay Time 3
    uint8_t rsvd134[122];
    uint16_t oacs;              // Optional Admin Command Support
    uint8_t acl;                // Abort Command Limit
    uint8_t aerl;               // Asynchronous Event Request Limit
    uint8_t frmw;               // Firmware Updates
    uint8_t lpa;                // Log Page Attributes
    uint8_t elpe;               // Error Log Page Entries
    uint8_t npss;               // Number of Power States Support
    uint8_t avscc;              // Admin Vendor Specific Command Configuration
    uint8_t apsta;              // Autonomous Power State Transition Attributes
    uint16_t wctemp;            // Warning Composite Temperature Threshold
    uint16_t cctemp;            // Critical Composite Temperature Threshold
    uint16_t mtfa;              // Maximum Time for Firmware Activation
    uint32_t hmpre;             // Host Memory Buffer Preferred Size
    uint32_t hmmin;             // Host Memory Buffer Minimum Size
    uint8_t tnvmcap[16];        // Total NVM Capacity
    uint8_t unvmcap[16];        // Unallocated NVM Capacity
    uint32_t rpmbs;             // Replay Protected Memory Block Support
    uint16_t edstt;             // Extended Device Self-test Time
    uint8_t dsto;               // Device Self-test Options
    uint8_t fwug;               // Firmware Update Granularity
    uint16_t kas;               // Keep Alive Support
    uint16_t hctma;             // Host Controlled Thermal Management Attributes
    uint16_t mntmt;             // Minimum Thermal Management Temperature
    uint16_t mxtmt;             // Maximum Thermal Management Temperature
    uint32_t sanicap;           // Sanitize Capabilities
    uint32_t hmminds;           // Host Memory Buffer Minimum Descriptor Entry Size
    uint16_t hmmaxd;            // Host Memory Buffer Maximum Descriptors Entries
    uint16_t nsetidmax;         // NVM Set Identifier Maximum
    uint16_t endgidmax;         // Endurance Group Identifier Maximum
    uint8_t anatt;              // ANA Transition Time
    uint8_t anacap;             // Asymmetric Namespace Access Capabilities
    uint32_t anagrpmax;         // ANA Group Identifier Maximum
    uint32_t nanagrpid;         // Number of ANA Group Identifiers
    uint32_t pels;              // Persistent Event Log Size
    uint16_t domainid;          // Domain Identifier
    uint8_t rsvd358[10];
    uint8_t megcap[16];         // Max Endurance Group Capacity
    uint8_t rsvd384[128];
    uint8_t sqes;               // Submission Queue Entry Size
    uint8_t cqes;               // Completion Queue Entry Size
    uint16_t maxcmd;            // Maximum Outstanding Commands
    uint32_t nn;                // Number of Namespaces
    uint16_t oncs;              // Optional NVM Command Support
    uint16_t fuses;             // Fused Operation Support
    uint8_t fna;                // Format NVM Attributes
    uint8_t vwc;                // Volatile Write Cache
    uint16_t awun;              // Atomic Write Unit Normal
    uint16_t awupf;             // Atomic Write Unit Power Fail
    uint8_t nvscc;              // NVM Vendor Specific Command Configuration
    uint8_t nwpc;               // Namespace Write Protection Capabilities
    uint16_t acwu;              // Atomic Compare & Write Unit
    uint16_t rsvd534;
    uint32_t sgls;              // SGL Support
    uint32_t mnan;              // Maximum Number of Allowed Namespaces
    uint8_t rsvd544[224];
    char subnqn[256];           // NVM Subsystem NVMe Qualified Name
    uint8_t rsvd1024[768];
    uint8_t nvmof[256];         // NVMe over Fabrics
    struct {
        uint16_t mp;            // Maximum Power
        uint8_t rsvd2;
        uint8_t mps : 1;        // Max Power Scale
        uint8_t nops : 1;       // Non-Operational State
        uint8_t rsvd3 : 6;
        uint32_t enlat;         // Entry Latency
        uint32_t exlat;         // Exit Latency
        uint8_t rrt : 5;        // Relative Read Throughput
        uint8_t rsvd4 : 3;
        uint8_t rrl : 5;        // Relative Read Latency
        uint8_t rsvd5 : 3;
        uint8_t rwt : 5;        // Relative Write Throughput
        uint8_t rsvd6 : 3;
        uint8_t rwl : 5;        // Relative Write Latency
        uint8_t rsvd7 : 3;
        uint16_t idlp;          // Idle Power
        uint8_t rsvd8 : 6;
        uint8_t ips : 2;        // Idle Power Scale
        uint8_t rsvd9;
        uint16_t actp;          // Active Power
        uint8_t apw : 3;        // Active Power Workload
        uint8_t rsvd10 : 3;
        uint8_t aps : 2;        // Active Power Scale
        uint8_t rsvd11[9];
    } psd[32];                  // Power State Descriptors
    uint8_t vs[1024];           // Vendor Specific
} __attribute__((packed)) nvme_id_ctrl_t;

// NVMe Identify Namespace Data Structure
typedef struct {
    uint64_t nsze;              // Namespace Size
    uint64_t ncap;              // Namespace Capacity
    uint64_t nuse;              // Namespace Utilization
    uint8_t nsfeat;             // Namespace Features
    uint8_t nlbaf;              // Number of LBA Formats
    uint8_t flbas;              // Formatted LBA Size
    uint8_t mc;                 // Metadata Capabilities
    uint8_t dpc;                // End-to-end Data Protection Capabilities
    uint8_t dps;                // End-to-end Data Protection Type Settings
    uint8_t nmic;               // Namespace Multi-path I/O and Namespace Sharing
    uint8_t rescap;             // Reservation Capabilities
    uint8_t fpi;                // Format Progress Indicator
    uint8_t dlfeat;             // Deallocate Logical Block Features
    uint16_t nawun;             // Namespace Atomic Write Unit Normal
    uint16_t nawupf;            // Namespace Atomic Write Unit Power Fail
    uint16_t nacwu;             // Namespace Atomic Compare & Write Unit
    uint16_t nabsn;             // Namespace Atomic Boundary Size Normal
    uint16_t nabo;              // Namespace Atomic Boundary Offset
    uint16_t nabspf;            // Namespace Atomic Boundary Size Power Fail
    uint16_t noiob;             // Namespace Optimal IO Boundary
    uint8_t nvmcap[16];         // NVM Capacity
    uint16_t npwg;              // Namespace Preferred Write Granularity
    uint16_t npwa;              // Namespace Preferred Write Alignment
    uint16_t npdg;              // Namespace Preferred Deallocate Granularity
    uint16_t npda;              // Namespace Preferred Deallocate Alignment
    uint16_t nows;              // Namespace Optimal Write Size
    uint16_t mssrl;             // Maximum Single Source Range Length
    uint32_t mcl;               // Maximum Copy Length
    uint8_t msrc;               // Maximum Source Range Count
    uint8_t rsvd81[11];
    uint32_t anagrpid;          // ANA Group Identifier
    uint8_t rsvd96[3];
    uint8_t nsattr;             // Namespace Attributes
    uint16_t nvmsetid;          // NVM Set Identifier
    uint16_t endgid;            // Endurance Group Identifier
    uint8_t nguid[16];          // Namespace Globally Unique Identifier
    uint8_t eui64[8];           // IEEE Extended Unique Identifier
    struct {
        uint16_t ms;            // Metadata Size
        uint8_t lbads;          // LBA Data Size
        uint8_t rp : 2;         // Relative Performance
        uint8_t rsvd : 6;
    } lbaf[16];                 // LBA Format Support
    uint8_t rsvd192[192];
    uint8_t vs[3712];           // Vendor Specific
} __attribute__((packed)) nvme_id_ns_t;

// NVMe Namespace Structure
struct nvme_namespace {
    nvme_controller_t* ctrl;    // Controller reference
    uint32_t nsid;              // Namespace ID
    nvme_id_ns_t* id;           // Identify data
    
    // Namespace properties
    uint64_t size;              // Size in logical blocks
    uint64_t capacity;          // Capacity in logical blocks
    uint32_t lba_size;          // Logical block size
    uint32_t metadata_size;     // Metadata size per block
    bool has_metadata;          // Metadata support
    
    // Protection information
    uint8_t pi_type;            // Protection Information type
    bool pi_first;              // PI at beginning of metadata
    
    // Performance characteristics
    uint32_t optimal_io_size;   // Optimal I/O size
    uint32_t atomic_write_unit; // Atomic write unit
    
    // Features
    bool supports_flush;        // Flush command support
    bool supports_write_zeroes; // Write Zeroes support
    bool supports_dsm;          // Dataset Management support
    bool supports_copy;         // Copy command support
    
    // Multi-path I/O
    uint32_t ana_group_id;      // ANA Group ID
    uint8_t ana_state;          // ANA State
    
    // Statistics
    struct {
        uint64_t read_commands;
        uint64_t write_commands;
        uint64_t bytes_read;
        uint64_t bytes_written;
        uint64_t errors;
    } stats;
    
    // Driver framework integration
    device_t* device_obj;
    
    // Synchronization
    void* lock;
};

// NVMe Controller Structure
struct nvme_controller {
    // Hardware information
    pci_device_t* pci_dev;      // PCI device
    void* bar;                  // Memory-mapped registers
    size_t bar_size;            // BAR size
    int irq;                    // Interrupt line
    
    // Controller capabilities
    nvme_id_ctrl_t* id;         // Identify data
    uint32_t version;           // NVMe specification version
    uint64_t cap;               // Controller capabilities
    uint32_t page_size;         // Memory page size
    uint32_t page_shift;        // Page size shift
    uint32_t max_hw_sectors;    // Maximum HW sectors per transfer
    uint32_t max_segments;      // Maximum SGL segments
    uint32_t max_integrity_segments; // Maximum PI segments
    
    // Queue management
    nvme_queue_t* admin_queue;  // Admin queue
    nvme_queue_t** io_queues;   // I/O queues array
    uint32_t queue_count;       // Number of I/O queues
    uint32_t max_qid;           // Maximum queue ID
    uint32_t io_queue_depth;    // I/O queue depth
    uint32_t admin_queue_depth; // Admin queue depth
    
    // Doorbell stride
    uint32_t db_stride;         // Doorbell stride
    volatile uint32_t* dbs;     // Doorbell registers
    
    // Namespace management
    nvme_namespace_t** namespaces; // Namespaces array
    uint32_t namespace_count;   // Number of namespaces
    uint32_t max_namespaces;    // Maximum namespaces
    
    // Features and capabilities
    bool supports_volatile_wc;  // Volatile write cache
    bool supports_host_mem_buf; // Host Memory Buffer
    bool supports_apst;         // Autonomous Power State Transition
    bool supports_streams;      // Streams
    bool supports_hmb;          // Host Memory Buffer
    bool supports_sgl;          // Scatter Gather Lists
    bool supports_pi;           // Protection Information
    bool supports_metadata;     // Metadata
    
    // Power management
    uint32_t power_state;       // Current power state
    uint32_t num_power_states;  // Number of power states
    bool apst_enabled;          // APST enabled
    
    // Thermal management
    uint16_t warning_temp;      // Warning temperature threshold
    uint16_t critical_temp;     // Critical temperature threshold
    uint16_t current_temp;      // Current temperature
    bool thermal_mgmt_enabled;  // Thermal management enabled
    
    // Host Memory Buffer
    struct {
        void* addr;             // HMB address
        size_t size;            // HMB size
        uint32_t chunk_size;    // Chunk size
        uint64_t* desc_list;    // Descriptor list
        uint64_t desc_list_dma; // Descriptor list DMA
        bool enabled;           // HMB enabled
    } hmb;
    
    // Performance optimization
    struct {
        uint32_t io_timeout;    // I/O timeout
        uint32_t admin_timeout; // Admin timeout
        bool polling_enabled;   // Polling mode
        uint32_t poll_queues;   // Number of poll queues
        uint32_t write_queues;  // Number of write queues
    } perf;
    
    // Statistics and monitoring
    struct {
        uint64_t commands_completed;
        uint64_t commands_failed;
        uint64_t bytes_transferred;
        uint32_t queue_depth_used;
        uint32_t temperature_events;
        uint64_t power_cycles;
        uint64_t unsafe_shutdowns;
    } stats;
    
    // Error handling
    struct {
        uint32_t error_count;
        uint32_t timeout_count;
        bool subsystem_reset_required;
    } error;
    
    // State management
    enum {
        NVME_CTRL_LIVE,
        NVME_CTRL_ADMIN_ONLY,
        NVME_CTRL_RESETTING,
        NVME_CTRL_CONNECTING,
        NVME_CTRL_DELETING,
        NVME_CTRL_DEAD
    } state;
    
    // Driver framework integration
    driver_t* driver;
    device_t* device_obj;
    
    // Synchronization
    void* lock;
    
    // Linked list
    nvme_controller_t* next;
};

// Function prototypes

// Core initialization and cleanup
int nvme_init(void);
void nvme_cleanup(void);

// Controller management
int nvme_probe_controller(pci_device_t* pci_dev);
int nvme_remove_controller(nvme_controller_t* ctrl);
int nvme_reset_controller(nvme_controller_t* ctrl);
int nvme_enable_controller(nvme_controller_t* ctrl);
int nvme_disable_controller(nvme_controller_t* ctrl);

// Queue management
int nvme_create_admin_queue(nvme_controller_t* ctrl);
int nvme_create_io_queues(nvme_controller_t* ctrl);
int nvme_delete_io_queues(nvme_controller_t* ctrl);
int nvme_create_queue(nvme_controller_t* ctrl, uint16_t qid, uint16_t size, 
                     nvme_queue_type_t type, uint16_t cq_vector);
int nvme_delete_queue(nvme_controller_t* ctrl, uint16_t qid, nvme_queue_type_t type);

// Command submission and completion
int nvme_submit_sync_cmd(nvme_queue_t* queue, nvme_command_t* cmd, 
                        uint32_t* result, uint32_t timeout);
int nvme_submit_async_cmd(nvme_queue_t* queue, nvme_request_t* req);
void nvme_complete_request(nvme_request_t* req, nvme_completion_t* cpl);
void nvme_process_cq(nvme_queue_t* queue);

// I/O operations
int nvme_read_sectors(nvme_namespace_t* ns, uint64_t lba, uint32_t num_sectors, 
                     void* buffer);
int nvme_write_sectors(nvme_namespace_t* ns, uint64_t lba, uint32_t num_sectors, 
                      const void* buffer);
int nvme_flush(nvme_namespace_t* ns);
int nvme_write_zeroes(nvme_namespace_t* ns, uint64_t lba, uint32_t num_sectors);
int nvme_dataset_management(nvme_namespace_t* ns, uint64_t lba, 
                           uint32_t num_sectors, uint32_t attributes);

// Admin commands
int nvme_identify_controller(nvme_controller_t* ctrl, nvme_id_ctrl_t* id);
int nvme_identify_namespace(nvme_controller_t* ctrl, uint32_t nsid, nvme_id_ns_t* id);
int nvme_get_features(nvme_controller_t* ctrl, uint8_t fid, uint32_t nsid, 
                     uint64_t data_addr, uint32_t* result);
int nvme_set_features(nvme_controller_t* ctrl, uint8_t fid, uint32_t nsid, 
                     uint32_t dword11, uint64_t data_addr, uint32_t* result);
int nvme_get_log_page(nvme_controller_t* ctrl, uint8_t log_id, uint32_t nsid, 
                     uint64_t lpo, void* buffer, size_t size);

// Namespace management
int nvme_scan_namespaces(nvme_controller_t* ctrl);
int nvme_add_namespace(nvme_controller_t* ctrl, uint32_t nsid);
int nvme_remove_namespace(nvme_controller_t* ctrl, uint32_t nsid);
nvme_namespace_t* nvme_find_namespace(nvme_controller_t* ctrl, uint32_t nsid);

// Power management
int nvme_set_power_state(nvme_controller_t* ctrl, uint32_t ps);
int nvme_enable_apst(nvme_controller_t* ctrl);
int nvme_configure_apst(nvme_controller_t* ctrl);

// Thermal management
int nvme_get_temperature(nvme_controller_t* ctrl, uint16_t* temp);
int nvme_set_temp_threshold(nvme_controller_t* ctrl, uint16_t temp, bool over);
int nvme_enable_thermal_mgmt(nvme_controller_t* ctrl);

// Host Memory Buffer
int nvme_setup_host_mem_buf(nvme_controller_t* ctrl);
int nvme_free_host_mem_buf(nvme_controller_t* ctrl);

// Performance optimization
int nvme_configure_interrupt_coalescing(nvme_controller_t* ctrl, 
                                       uint8_t threshold, uint8_t time);
int nvme_enable_polling_mode(nvme_controller_t* ctrl, uint32_t poll_queues);
int nvme_optimize_queue_depth(nvme_controller_t* ctrl);
int nvme_enable_write_cache(nvme_controller_t* ctrl, bool enable);

// Error handling and recovery
int nvme_handle_cqe_error(nvme_controller_t* ctrl, nvme_completion_t* cpl);
int nvme_abort_command(nvme_controller_t* ctrl, uint16_t sqid, uint16_t cid);
int nvme_reset_subsystem(nvme_controller_t* ctrl);

// Diagnostics and monitoring
int nvme_self_test(nvme_controller_t* ctrl, uint8_t stc);
int nvme_get_smart_log(nvme_controller_t* ctrl, uint32_t nsid, void* log);
int nvme_get_error_log(nvme_controller_t* ctrl, void* log, uint32_t entries);
void nvme_print_controller_info(nvme_controller_t* ctrl);
void nvme_print_namespace_info(nvme_namespace_t* ns);

// Utility functions
const char* nvme_status_to_string(uint16_t status);
const char* nvme_opcode_to_string(uint8_t opcode);
uint32_t nvme_get_max_transfer_size(nvme_controller_t* ctrl);
bool nvme_is_ready(nvme_controller_t* ctrl);
uint64_t nvme_lba_to_sector(nvme_namespace_t* ns, uint64_t lba);
uint64_t nvme_sector_to_lba(nvme_namespace_t* ns, uint64_t sector);

// Legacy wrapper functions for compatibility
int nvme_read_sectors_legacy(uint8_t drive, uint64_t lba, uint32_t num_sectors, uint8_t* buffer);
int nvme_write_sectors_legacy(uint8_t drive, uint64_t lba, uint32_t num_sectors, const uint8_t* buffer);
int nvme_submit_command(uint8_t drive, const nvme_command_t* cmd);
int nvme_poll_completion(uint8_t drive, nvme_completion_t* completion);

// Error codes
#define NVME_SUCCESS            0
#define NVME_ERR_NO_DEVICE     -5001
#define NVME_ERR_NO_MEMORY     -5002
#define NVME_ERR_TIMEOUT       -5003
#define NVME_ERR_IO            -5004
#define NVME_ERR_PROTOCOL      -5005
#define NVME_ERR_NOT_SUPPORTED -5006
#define NVME_ERR_CONTROLLER    -5007
#define NVME_ERR_NAMESPACE     -5008
#define NVME_ERR_QUEUE_FULL    -5009
#define NVME_ERR_ABORT         -5010

#ifdef __cplusplus
}
#endif

#endif // NVME_H

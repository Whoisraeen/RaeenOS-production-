/**
 * RaeenOS Production NVMe Driver
 * High-performance NVMe storage driver with queue management
 */

#include "nvme_driver.h"
#include "../memory.h"
#include "../pci.h"
#include "../interrupts.h"

// NVMe Controller Registers
#define NVME_REG_CAP    0x00    // Controller Capabilities
#define NVME_REG_VS     0x08    // Version
#define NVME_REG_INTMS  0x0C    // Interrupt Mask Set
#define NVME_REG_INTMC  0x10    // Interrupt Mask Clear
#define NVME_REG_CC     0x14    // Controller Configuration
#define NVME_REG_CSTS   0x1C    // Controller Status
#define NVME_REG_AQA    0x24    // Admin Queue Attributes
#define NVME_REG_ASQ    0x28    // Admin Submission Queue Base
#define NVME_REG_ACQ    0x30    // Admin Completion Queue Base

// NVMe Command Opcodes
#define NVME_ADMIN_DELETE_SQ    0x00
#define NVME_ADMIN_CREATE_SQ    0x01
#define NVME_ADMIN_DELETE_CQ    0x04
#define NVME_ADMIN_CREATE_CQ    0x05
#define NVME_ADMIN_IDENTIFY     0x06
#define NVME_CMD_READ           0x02
#define NVME_CMD_WRITE          0x01

// NVMe Structures
typedef struct {
    uint32_t cdw0;
    uint32_t nsid;
    uint64_t reserved;
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} nvme_command_t;

typedef struct {
    uint32_t result;
    uint32_t reserved;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
} nvme_completion_t;

typedef struct {
    nvme_command_t* commands;
    volatile uint32_t* doorbell;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint16_t id;
} nvme_queue_t;

typedef struct {
    nvme_completion_t* completions;
    volatile uint32_t* doorbell;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint16_t id;
    uint8_t phase;
} nvme_cqueue_t;

// NVMe Device Structure
struct nvme_device {
    volatile uint8_t* mmio_base;
    uint32_t stride;
    uint16_t max_queue_entries;
    uint16_t timeout;
    
    nvme_queue_t admin_sq;
    nvme_cqueue_t admin_cq;
    nvme_queue_t io_sq;
    nvme_cqueue_t io_cq;
    
    uint32_t namespace_count;
    uint64_t namespace_size;
    uint32_t block_size;
    
    bool initialized;
    uint16_t command_id;
};

static nvme_device_t g_nvme_device = {0};

// Function declarations
static bool nvme_wait_ready(nvme_device_t* dev, bool ready);
static bool nvme_reset_controller(nvme_device_t* dev);
static bool nvme_setup_admin_queues(nvme_device_t* dev);
static bool nvme_setup_io_queues(nvme_device_t* dev);
static bool nvme_identify_controller(nvme_device_t* dev);
static bool nvme_identify_namespace(nvme_device_t* dev, uint32_t nsid);
static uint16_t nvme_submit_command(nvme_queue_t* sq, nvme_command_t* cmd);
static bool nvme_wait_completion(nvme_cqueue_t* cq, uint16_t cid, nvme_completion_t* result);
static void nvme_ring_doorbell(volatile uint32_t* doorbell, uint16_t value);

/**
 * Initialize NVMe driver
 */
bool nvme_init(void) {
    nvme_device_t* dev = &g_nvme_device;
    
    // Find NVMe controller via PCI
    pci_device_t pci_dev;
    if (!pci_find_device(0x01, 0x08, 0x02, &pci_dev)) { // NVMe class/subclass/interface
        printf("NVMe: No NVMe controller found\n");
        return false;
    }
    
    // Map MMIO region
    uint64_t bar0 = pci_read_config(&pci_dev, 0x10);
    if (!(bar0 & 0x1)) { // Memory BAR
        dev->mmio_base = (volatile uint8_t*)(bar0 & ~0xF);
        printf("NVMe: Controller at 0x%p\n", dev->mmio_base);
    } else {
        printf("NVMe: Invalid BAR0\n");
        return false;
    }
    
    // Enable PCI device
    uint16_t command = pci_read_config_16(&pci_dev, 0x04);
    command |= 0x06; // Memory space + bus master
    pci_write_config_16(&pci_dev, 0x04, command);
    
    // Read controller capabilities
    uint64_t cap = *(volatile uint64_t*)(dev->mmio_base + NVME_REG_CAP);
    dev->max_queue_entries = ((cap >> 16) & 0xFFFF) + 1;
    dev->stride = 1 << (((cap >> 32) & 0xF) + 2);
    dev->timeout = ((cap >> 24) & 0xFF) * 500; // Convert to ms
    
    printf("NVMe: Max queue entries: %u, stride: %u, timeout: %u ms\n",
           dev->max_queue_entries, dev->stride, dev->timeout);
    
    // Reset controller
    if (!nvme_reset_controller(dev)) {
        printf("NVMe: Controller reset failed\n");
        return false;
    }
    
    // Setup admin queues
    if (!nvme_setup_admin_queues(dev)) {
        printf("NVMe: Admin queue setup failed\n");
        return false;
    }
    
    // Identify controller
    if (!nvme_identify_controller(dev)) {
        printf("NVMe: Controller identify failed\n");
        return false;
    }
    
    // Setup I/O queues
    if (!nvme_setup_io_queues(dev)) {
        printf("NVMe: I/O queue setup failed\n");
        return false;
    }
    
    // Identify namespace 1
    if (!nvme_identify_namespace(dev, 1)) {
        printf("NVMe: Namespace identify failed\n");
        return false;
    }
    
    dev->initialized = true;
    printf("NVMe: Driver initialized successfully\n");
    printf("NVMe: Namespace size: %llu sectors (%u bytes/sector)\n",
           dev->namespace_size, dev->block_size);
    
    return true;
}

/**
 * Read sectors from NVMe device
 */
bool nvme_read_sectors(nvme_device_t* dev, uint64_t lba, uint32_t count, void* buffer) {
    if (!dev || !dev->initialized || !buffer || count == 0) return false;
    
    // Prepare read command
    nvme_command_t cmd = {0};
    cmd.cdw0 = NVME_CMD_READ;
    cmd.nsid = 1; // Namespace 1
    cmd.prp1 = (uint64_t)buffer;
    
    // Handle PRP2 for transfers > 4KB
    if (count * dev->block_size > 4096) {
        cmd.prp2 = (uint64_t)buffer + 4096;
    }
    
    cmd.cdw10 = (uint32_t)lba;
    cmd.cdw11 = (uint32_t)(lba >> 32);
    cmd.cdw12 = (count - 1); // 0-based count
    
    // Submit command
    uint16_t cid = nvme_submit_command(&dev->io_sq, &cmd);
    if (cid == 0) return false;
    
    // Wait for completion
    nvme_completion_t completion;
    if (!nvme_wait_completion(&dev->io_cq, cid, &completion)) {
        return false;
    }
    
    return (completion.status & 0x7FF) == 0; // Check status field
}

/**
 * Write sectors to NVMe device
 */
bool nvme_write_sectors(nvme_device_t* dev, uint64_t lba, uint32_t count, const void* buffer) {
    if (!dev || !dev->initialized || !buffer || count == 0) return false;
    
    nvme_command_t cmd = {0};
    cmd.cdw0 = NVME_CMD_WRITE;
    cmd.nsid = 1;
    cmd.prp1 = (uint64_t)buffer;
    
    if (count * dev->block_size > 4096) {
        cmd.prp2 = (uint64_t)buffer + 4096;
    }
    
    cmd.cdw10 = (uint32_t)lba;
    cmd.cdw11 = (uint32_t)(lba >> 32);
    cmd.cdw12 = (count - 1);
    
    uint16_t cid = nvme_submit_command(&dev->io_sq, &cmd);
    if (cid == 0) return false;
    
    nvme_completion_t completion;
    if (!nvme_wait_completion(&dev->io_cq, cid, &completion)) {
        return false;
    }
    
    return (completion.status & 0x7FF) == 0;
}

/**
 * Get NVMe device handle
 */
nvme_device_t* nvme_get_device(void) {
    return g_nvme_device.initialized ? &g_nvme_device : NULL;
}

// Internal helper functions

static bool nvme_wait_ready(nvme_device_t* dev, bool ready) {
    uint32_t timeout = dev->timeout;
    
    while (timeout--) {
        uint32_t csts = *(volatile uint32_t*)(dev->mmio_base + NVME_REG_CSTS);
        bool current_ready = (csts & 0x1) != 0;
        
        if (current_ready == ready) {
            return true;
        }
        
        // Wait 1ms
        for (volatile int i = 0; i < 1000000; i++);
    }
    
    return false;
}

static bool nvme_reset_controller(nvme_device_t* dev) {
    // Disable controller
    *(volatile uint32_t*)(dev->mmio_base + NVME_REG_CC) = 0;
    
    // Wait for not ready
    if (!nvme_wait_ready(dev, false)) {
        return false;
    }
    
    // Configure controller
    uint32_t cc = 0;
    cc |= (0x6 << 16); // I/O Submission Queue Entry Size (64 bytes)
    cc |= (0x4 << 20); // I/O Completion Queue Entry Size (16 bytes)
    cc |= (0x0 << 11); // Memory Page Size (4KB)
    cc |= 0x1;         // Enable
    
    *(volatile uint32_t*)(dev->mmio_base + NVME_REG_CC) = cc;
    
    // Wait for ready
    return nvme_wait_ready(dev, true);
}

static bool nvme_setup_admin_queues(nvme_device_t* dev) {
    // Allocate admin queues
    size_t sq_size = 64 * 64; // 64 entries * 64 bytes
    size_t cq_size = 64 * 16; // 64 entries * 16 bytes
    
    dev->admin_sq.commands = (nvme_command_t*)memory_alloc_aligned(sq_size, 4096);
    dev->admin_cq.completions = (nvme_completion_t*)memory_alloc_aligned(cq_size, 4096);
    
    if (!dev->admin_sq.commands || !dev->admin_cq.completions) {
        return false;
    }
    
    memory_set(dev->admin_sq.commands, 0, sq_size);
    memory_set(dev->admin_cq.completions, 0, cq_size);
    
    dev->admin_sq.size = 64;
    dev->admin_sq.head = 0;
    dev->admin_sq.tail = 0;
    dev->admin_sq.id = 0;
    
    dev->admin_cq.size = 64;
    dev->admin_cq.head = 0;
    dev->admin_cq.tail = 0;
    dev->admin_cq.id = 0;
    dev->admin_cq.phase = 1;
    
    // Set queue attributes
    uint32_t aqa = ((64 - 1) << 16) | (64 - 1); // CQ size | SQ size
    *(volatile uint32_t*)(dev->mmio_base + NVME_REG_AQA) = aqa;
    
    // Set queue base addresses
    *(volatile uint64_t*)(dev->mmio_base + NVME_REG_ASQ) = (uint64_t)dev->admin_sq.commands;
    *(volatile uint64_t*)(dev->mmio_base + NVME_REG_ACQ) = (uint64_t)dev->admin_cq.completions;
    
    // Set doorbell pointers
    dev->admin_sq.doorbell = (volatile uint32_t*)(dev->mmio_base + 0x1000);
    dev->admin_cq.doorbell = (volatile uint32_t*)(dev->mmio_base + 0x1000 + dev->stride);
    
    return true;
}

static uint16_t nvme_submit_command(nvme_queue_t* sq, nvme_command_t* cmd) {
    uint16_t cid = ++g_nvme_device.command_id;
    cmd->cdw0 |= (cid << 16);
    
    // Copy command to queue
    memory_copy(&sq->commands[sq->tail], cmd, sizeof(nvme_command_t));
    
    // Update tail
    sq->tail = (sq->tail + 1) % sq->size;
    
    // Ring doorbell
    nvme_ring_doorbell(sq->doorbell, sq->tail);
    
    return cid;
}

static bool nvme_wait_completion(nvme_cqueue_t* cq, uint16_t cid, nvme_completion_t* result) {
    uint32_t timeout = 1000; // 1 second timeout
    
    while (timeout--) {
        nvme_completion_t* entry = &cq->completions[cq->head];
        
        // Check phase bit
        if (((entry->status >> 15) & 1) == cq->phase) {
            if ((entry->cid & 0xFFFF) == cid) {
                if (result) {
                    memory_copy(result, entry, sizeof(nvme_completion_t));
                }
                
                // Update head
                cq->head = (cq->head + 1) % cq->size;
                if (cq->head == 0) {
                    cq->phase = !cq->phase;
                }
                
                // Ring doorbell
                nvme_ring_doorbell(cq->doorbell, cq->head);
                
                return true;
            }
        }
        
        // Wait 1ms
        for (volatile int i = 0; i < 1000000; i++);
    }
    
    return false;
}

static void nvme_ring_doorbell(volatile uint32_t* doorbell, uint16_t value) {
    *doorbell = value;
}

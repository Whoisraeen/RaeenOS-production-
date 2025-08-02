#include "nvme.h"
#include "../../kernel/vga.h"
#include "../../kernel/include/driver.h"
#include "../../libs/libc/include/string.h"

// NVMe driver structure
static driver_t nvme_driver = {
    .name = "NVMe Driver",
    .init = nvme_init,
    .probe = NULL // Not a bus driver
};

void nvme_init(void) {
    debug_print("NVMe driver initialized (placeholder).\n");
}

int nvme_read_sectors(uint8_t drive, uint64_t lba, uint32_t num_sectors, uint8_t* buffer) {
    (void)drive;
    (void)lba;
    (void)num_sectors;
    (void)buffer;
    debug_print("NVMe: Reading sectors (simulated).\n");
    return 0; // Success
}

int nvme_write_sectors(uint8_t drive, uint64_t lba, uint32_t num_sectors, const uint8_t* buffer) {
    (void)drive;
    (void)lba;
    (void)num_sectors;
    (void)buffer;
    debug_print("NVMe: Writing sectors (simulated).\n");
    return 0; // Success
}

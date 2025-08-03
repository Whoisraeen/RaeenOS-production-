#include "include/driver.h"
#include "vga.h"

#define MAX_DRIVERS 32

static driver_t* registered_drivers[MAX_DRIVERS];
static uint32_t num_registered_drivers = 0;

void register_driver(driver_t* driver) {
    if (num_registered_drivers >= MAX_DRIVERS) {
        vga_puts("Error: Too many drivers registered!\n");
        return;
    }
    registered_drivers[num_registered_drivers++] = driver;
    vga_puts("Driver registered: ");
    vga_puts(driver->name);
    vga_puts("\n");
}

// You might add functions here to iterate through registered drivers
// or to find a driver by name/type, etc.


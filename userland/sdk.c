#include "sdk.h"
#include "../kernel/vga.h"

void raeenos_sdk_init(void) {
    vga_puts("RaeenOS SDK initialized (placeholder).\n");
}

int raeenos_sdk_get_system_info(void* info_buffer, uint32_t buffer_size) {
    (void)info_buffer;
    (void)buffer_size;
    vga_puts("Getting system info via SDK (placeholder).\n");
    return -1; // Not implemented
}

int raeenos_sdk_register_app(const char* app_manifest_path) {
    (void)app_manifest_path;
    vga_puts("Registering app via SDK (placeholder).\n");
    return -1; // Not implemented
}


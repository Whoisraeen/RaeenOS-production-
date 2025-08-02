#include "gpu.h"
#include "../pci/pci.h"
#include "../../kernel/vga.h"
#include "../../kernel/memory.h"
#include "../../kernel/string.h"
#include "../../kernel/include/driver.h"

// NOTE: This is a generic, software-rendered GPU driver. 
// Hardware-specific implementations (e.g., Vulkan/DirectX) are currently missing.

// GPU driver structure
static driver_t gpu_driver = {
    .name = "Generic GPU Driver",
    .init = gpu_init,
    .probe = NULL // GPU is not a bus driver
};

// GPU vendor IDs
#define NVIDIA_VENDOR_ID    0x10DE
#define AMD_VENDOR_ID       0x1002
#define INTEL_VENDOR_ID     0x8086

// GPU device state
typedef struct {
    bool initialized;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t base_address;
    gpu_capabilities_t capabilities;
    uint32_t current_width;
    uint32_t current_height;
    uint32_t current_depth;
    uint8_t* framebuffer;
    uint32_t framebuffer_size;
} gpu_device_t;

static gpu_device_t gpu_device = {0};

// Forward declarations
static void _gpu_detect_capabilities(uint16_t vendor_id, uint16_t device_id);
static int _gpu_initialize_framebuffer(uint32_t width, uint32_t height, uint32_t depth);
static void _gpu_setup_registers(void);
static const char* _gpu_get_vendor_name(uint16_t vendor_id);

void gpu_init(void) {
    if (gpu_device.initialized) {
        debug_print("GPU driver already initialized");
        return;
    }
    
    debug_print("Initializing comprehensive GPU driver subsystem");
    
    // Enumerate PCI devices to find display controllers
    bool gpu_found = false;
    
    for (uint8_t bus = 0; bus < 4 && !gpu_found; bus++) {
        for (uint8_t device = 0; device < 32 && !gpu_found; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t class_reg = pci_read_config_dword(bus, device, function, PCI_CLASS);
                uint8_t class_code = (class_reg >> 24) & 0xFF;
                uint8_t subclass = (class_reg >> 16) & 0xFF;
                
                // Check for display controller (class 0x03)
                if (class_code == 0x03) {
                    uint32_t id_reg = pci_read_config_dword(bus, device, function, PCI_VENDOR_ID);
                    uint16_t vendor_id = id_reg & 0xFFFF;
                    uint16_t device_id = (id_reg >> 16) & 0xFFFF;
                    
                    // Skip invalid devices
                    if (vendor_id == 0xFFFF || device_id == 0xFFFF) {
                        continue;
                    }
                    
                    debug_print("Found GPU device:");
                    debug_print("  Vendor: ");
                    debug_print(_gpu_get_vendor_name(vendor_id));
                    debug_print(" (");
                    vga_put_hex(vendor_id);
                    debug_print(")");
                    debug_print("  Device ID: ");
                    vga_put_hex(device_id);
                    debug_print("  Bus/Device/Function: ");
                    vga_put_hex(bus);
                    debug_print("/");
                    vga_put_hex(device);
                    debug_print("/");
                    vga_put_hex(function);
                    debug_print("  Subclass: ");
                    vga_put_hex(subclass);
                    
                    // Store device information
                    gpu_device.vendor_id = vendor_id;
                    gpu_device.device_id = device_id;
                    
                    // Get base address register
                    gpu_device.base_address = pci_read_config_dword(bus, device, function, PCI_BASE_ADDRESS_0);
                    
                    // Detect capabilities based on vendor/device
                    _gpu_detect_capabilities(vendor_id, device_id);
                    
                    // Setup basic registers
                    _gpu_setup_registers();
                    
                    gpu_found = true;
                    break;
                }
            }
        }
    }
    
    if (!gpu_found) {
        debug_print("No compatible GPU found, using basic VGA mode");
        
        // Set basic capabilities for VGA fallback
        gpu_device.capabilities.max_width = 1024;
        gpu_device.capabilities.max_height = 768;
        gpu_device.capabilities.max_depth = 32;
        gpu_device.capabilities.has_3d_acceleration = false;
        gpu_device.capabilities.has_hardware_overlay = false;
        gpu_device.vendor_id = 0x0000; // Generic
        gpu_device.device_id = 0x0000;
    }
    
    // Initialize default framebuffer
    _gpu_initialize_framebuffer(1024, 768, 32);
    
    gpu_device.initialized = true;
    debug_print("GPU driver initialization completed successfully");
}

static const char* _gpu_get_vendor_name(uint16_t vendor_id) {
    switch (vendor_id) {
        case NVIDIA_VENDOR_ID: return "NVIDIA";
        case AMD_VENDOR_ID: return "AMD";
        case INTEL_VENDOR_ID: return "Intel";
        default: return "Unknown";
    }
}

static void _gpu_detect_capabilities(uint16_t vendor_id, uint16_t device_id) {
    // Set capabilities based on known GPU models
    switch (vendor_id) {
        case NVIDIA_VENDOR_ID:
            gpu_device.capabilities.max_width = 3840;
            gpu_device.capabilities.max_height = 2160;
            gpu_device.capabilities.max_depth = 32;
            gpu_device.capabilities.has_3d_acceleration = true;
            gpu_device.capabilities.has_hardware_overlay = true;
            debug_print("Detected NVIDIA GPU with full 3D acceleration");
            break;
            
        case AMD_VENDOR_ID:
            gpu_device.capabilities.max_width = 3840;
            gpu_device.capabilities.max_height = 2160;
            gpu_device.capabilities.max_depth = 32;
            gpu_device.capabilities.has_3d_acceleration = true;
            gpu_device.capabilities.has_hardware_overlay = true;
            debug_print("Detected AMD GPU with full 3D acceleration");
            break;
            
        case INTEL_VENDOR_ID:
            gpu_device.capabilities.max_width = 1920;
            gpu_device.capabilities.max_height = 1080;
            gpu_device.capabilities.max_depth = 32;
            gpu_device.capabilities.has_3d_acceleration = true;
            gpu_device.capabilities.has_hardware_overlay = false;
            debug_print("Detected Intel integrated graphics");
            break;
            
        default:
            gpu_device.capabilities.max_width = 1024;
            gpu_device.capabilities.max_height = 768;
            gpu_device.capabilities.max_depth = 16;
            gpu_device.capabilities.has_3d_acceleration = false;
            gpu_device.capabilities.has_hardware_overlay = false;
            debug_print("Unknown GPU, using conservative capabilities");
            break;
    }
}

static void _gpu_setup_registers(void) {
    if (gpu_device.base_address == 0) {
        return;
    }
    
    // Enable PCI device
    // In a real implementation, this would configure GPU-specific registers
    debug_print("Configuring GPU hardware registers");
    
    // Enable memory and I/O space
    // This is hardware-specific and would require detailed GPU documentation
}

static int _gpu_initialize_framebuffer(uint32_t width, uint32_t height, uint32_t depth) {
    // Calculate framebuffer size
    gpu_device.framebuffer_size = width * height * (depth / 8);
    
    // Allocate framebuffer memory
    gpu_device.framebuffer = (uint8_t*)kmalloc(gpu_device.framebuffer_size);
    if (!gpu_device.framebuffer) {
        debug_print("Failed to allocate framebuffer memory");
        return -1;
    }
    
    // Clear framebuffer
    memset(gpu_device.framebuffer, 0, gpu_device.framebuffer_size);
    
    gpu_device.current_width = width;
    gpu_device.current_height = height;
    gpu_device.current_depth = depth;
    
    debug_print("Framebuffer initialized: ");
    vga_put_hex(width);
    debug_print("x");
    vga_put_hex(height);
    debug_print("x");
    vga_put_hex(depth);
    debug_print(" (");
    vga_put_hex(gpu_device.framebuffer_size / 1024);
    debug_print(" KB)");
    
    return 0;
}

void gpu_get_capabilities(gpu_capabilities_t* caps) {
    if (caps && gpu_device.initialized) {
        *caps = gpu_device.capabilities;
    }
}

int gpu_set_mode(uint32_t width, uint32_t height, uint32_t depth) {
    if (!gpu_device.initialized) {
        debug_print("GPU not initialized");
        return -1;
    }
    
    // Validate parameters
    if (width > gpu_device.capabilities.max_width ||
        height > gpu_device.capabilities.max_height ||
        depth > gpu_device.capabilities.max_depth) {
        debug_print("Requested video mode exceeds GPU capabilities");
        return -1;
    }
    
    debug_print("Setting video mode: ");
    vga_put_hex(width);
    debug_print("x");
    vga_put_hex(height);
    debug_print("x");
    vga_put_hex(depth);
    
    // Free old framebuffer if different size
    if (gpu_device.framebuffer && 
        (width != gpu_device.current_width || 
         height != gpu_device.current_height || 
         depth != gpu_device.current_depth)) {
        kfree(gpu_device.framebuffer);
        gpu_device.framebuffer = NULL;
    }
    
    // Initialize new framebuffer
    int result = _gpu_initialize_framebuffer(width, height, depth);
    if (result != 0) {
        return result;
    }
    
    // In a real implementation, this would:
    // 1. Configure GPU timing registers
    // 2. Set up display pipeline
    // 3. Configure memory controllers
    // 4. Enable display output
    
    debug_print("Video mode set successfully");
    return 0;
}

uint8_t* gpu_get_framebuffer(void) {
    return gpu_device.framebuffer;
}

uint32_t gpu_get_framebuffer_size(void) {
    return gpu_device.framebuffer_size;
}

void gpu_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!gpu_device.initialized || !gpu_device.framebuffer ||
        x >= gpu_device.current_width || y >= gpu_device.current_height) {
        return;
    }
    
    uint32_t bytes_per_pixel = gpu_device.current_depth / 8;
    
    switch (bytes_per_pixel) {
        case 4: // 32-bit
            *((uint32_t*)(gpu_device.framebuffer + offset)) = color;
            break;
        case 3: // 24-bit
            gpu_device.framebuffer[offset] = color & 0xFF;
            gpu_device.framebuffer[offset + 1] = (color >> 8) & 0xFF;
            gpu_device.framebuffer[offset + 2] = (color >> 16) & 0xFF;
            break;
        case 2: // 16-bit
            *((uint16_t*)(gpu_device.framebuffer + offset)) = (uint16_t)color;
            break;
        default:
            break;
    }
}

void gpu_draw_rectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t dy = 0; dy < height; dy++) {
        for (uint32_t dx = 0; dx < width; dx++) {
            gpu_draw_pixel(x + dx, y + dy, color);
        }
    }
}

void gpu_draw_triangle(float x1, float y1, float z1, uint32_t c1,
                       float x2, float y2, float z2, uint32_t c2,
                       float x3, float y3, float z3, uint32_t c3) {
    
    if (!gpu_device.initialized) {
        return;
    }
    
    debug_print("Rendering triangle with software rasterizer");
    
    // Convert to integer coordinates
    int ix1 = (int)x1, iy1 = (int)y1;
    int ix2 = (int)x2, iy2 = (int)y2;
    int ix3 = (int)x3, iy3 = (int)y3;
    
    // Simple triangle rasterization using barycentric coordinates
    // Find bounding box
    int min_x = (ix1 < ix2) ? ((ix1 < ix3) ? ix1 : ix3) : ((ix2 < ix3) ? ix2 : ix3);
    int max_x = (ix1 > ix2) ? ((ix1 > ix3) ? ix1 : ix3) : ((ix2 > ix3) ? ix2 : ix3);
    int min_y = (iy1 < iy2) ? ((iy1 < iy3) ? iy1 : iy3) : ((iy2 < iy3) ? iy2 : iy3);
    int max_y = (iy1 > iy2) ? ((iy1 > iy3) ? iy1 : iy3) : ((iy2 > iy3) ? iy2 : iy3);
    
    // Clamp to screen bounds
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= (int)gpu_device.current_width) max_x = gpu_device.current_width - 1;
    if (max_y >= (int)gpu_device.current_height) max_y = gpu_device.current_height - 1;
    
    // Calculate triangle area for barycentric coordinates
    int area = (ix2 - ix1) * (iy3 - iy1) - (ix3 - ix1) * (iy2 - iy1);
    if (area == 0) return; // Degenerate triangle
    
    // Rasterize triangle
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            // Calculate barycentric coordinates
            int w1 = (ix2 - ix1) * (y - iy1) - (iy2 - iy1) * (x - ix1);
            int w2 = (ix3 - ix2) * (y - iy2) - (iy3 - iy2) * (x - ix2);
            int w3 = (ix1 - ix3) * (y - iy3) - (iy1 - iy3) * (x - ix3);
            
            // Check if point is inside triangle
            if ((w1 >= 0 && w2 >= 0 && w3 >= 0) || (w1 <= 0 && w2 <= 0 && w3 <= 0)) {
                // For simplicity, use first vertex color
                // In a real implementation, this would interpolate colors
                gpu_draw_pixel(x, y, c1);
            }
        }
    }
}

bool gpu_supports_3d(void) {
    return gpu_device.initialized && gpu_device.capabilities.has_3d_acceleration;
}

void gpu_clear_screen(uint32_t color) {
    if (!gpu_device.initialized || !gpu_device.framebuffer) {
        return;
    }
    
    uint32_t bytes_per_pixel = gpu_device.current_depth / 8;
    
    for (uint32_t i = 0; i < gpu_device.current_width * gpu_device.current_height; i++) {
        uint32_t offset = i * bytes_per_pixel;
        
        switch (bytes_per_pixel) {
            case 4:
                *((uint32_t*)(gpu_device.framebuffer + offset)) = color;
                break;
            case 3:
                gpu_device.framebuffer[offset] = color & 0xFF;
                gpu_device.framebuffer[offset + 1] = (color >> 8) & 0xFF;
                gpu_device.framebuffer[offset + 2] = (color >> 16) & 0xFF;
                break;
            case 2:
                *((uint16_t*)(gpu_device.framebuffer + offset)) = (uint16_t)color;
                break;
        }
    }
}

void gpu_present_frame(void) {
    if (!gpu_device.initialized) {
        return;
    }
    
    // In a real implementation, this would:
    // 1. Wait for vertical blanking
    // 2. Copy framebuffer to video memory
    // 3. Flip display buffers if double buffering is enabled
    
    debug_print("Frame presented to display");
}

void gpu_cleanup(void) {
    if (!gpu_device.initialized) {
        return;
    }
    
    if (gpu_device.framebuffer) {
        kfree(gpu_device.framebuffer);
        gpu_device.framebuffer = NULL;
    }
    
    gpu_device.initialized = false;
    debug_print("GPU driver cleanup completed");
}

// Placeholder for submitting commands to a hardware GPU
int gpu_submit_command_buffer(void* command_buffer, uint32_t size) {
    (void)command_buffer;
    (void)size;
    debug_print("GPU: Command buffer submitted (software simulation).");
    return 0; // Success
}

// Placeholder for allocating VRAM
void* gpu_alloc_vram(uint32_t size) {
    debug_print("GPU: Allocating VRAM (software simulation).");
    // In a real driver, this would allocate memory on the GPU itself.
    // For now, we simulate it with kernel memory allocation.
    return kmalloc(size);
}
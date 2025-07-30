#include "../unit/test_framework.h"
#include "../../kernel/driver.h"
#include "../../drivers/pci/pci.h"
#include "../../drivers/gpu/gpu.h"
#include "../../drivers/audio/audio.h"
#include <stdint.h>

// Integration tests for kernel-driver interactions

DEFINE_TEST(test_driver_registration, "Test driver registration and enumeration", "integration", false) {
    // Test driver registration system
    driver_info_t test_driver = {
        .name = "test_driver",
        .version = "1.0.0",
        .type = DRIVER_TYPE_BLOCK,
        .init = test_driver_init,
        .cleanup = test_driver_cleanup,
        .probe = test_driver_probe
    };
    
    // Register the test driver
    int result = register_driver(&test_driver);
    TEST_ASSERT_EQ(0, result, "Driver registration should succeed");
    
    // Verify driver is registered
    driver_info_t* found_driver = find_driver_by_name("test_driver");
    TEST_ASSERT_NOT_NULL(found_driver, "Registered driver should be findable");
    TEST_ASSERT_EQ(DRIVER_TYPE_BLOCK, found_driver->type, "Driver type should match");
    
    // Test driver enumeration
    int driver_count = get_registered_driver_count();
    TEST_ASSERT(driver_count > 0, "Should have at least one registered driver");
    
    // Unregister driver
    result = unregister_driver("test_driver");
    TEST_ASSERT_EQ(0, result, "Driver unregistration should succeed");
    
    return TEST_PASS;
}

DEFINE_TEST(test_pci_device_discovery, "Test PCI device discovery and driver matching", "integration", true) {
    // Initialize PCI subsystem
    int result = pci_init();
    TEST_ASSERT_EQ(0, result, "PCI initialization should succeed");
    
    // Scan for PCI devices
    int device_count = pci_scan_devices();
    TEST_ASSERT(device_count >= 0, "PCI scan should complete successfully");
    
    // Test device enumeration
    for (int i = 0; i < device_count && i < 10; i++) { // Test first 10 devices
        pci_device_t* device = pci_get_device(i);
        TEST_ASSERT_NOT_NULL(device, "PCI device should be valid");
        TEST_ASSERT(device->vendor_id != 0xFFFF, "Device should have valid vendor ID");
        TEST_ASSERT(device->device_id != 0xFFFF, "Device should have valid device ID");
        
        // Test driver matching
        driver_info_t* driver = find_driver_for_pci_device(device);
        if (driver) {
            printf("  Found driver '%s' for PCI device %04X:%04X\n", 
                   driver->name, device->vendor_id, device->device_id);
        }
    }
    
    return TEST_PASS;
}

DEFINE_TEST(test_gpu_driver_integration, "Test GPU driver integration with kernel", "integration", true) {
    // Test GPU driver loading and initialization
    int result = gpu_subsystem_init();
    TEST_ASSERT_EQ(0, result, "GPU subsystem initialization should succeed");
    
    // Discover GPU devices
    int gpu_count = gpu_discover_devices();
    TEST_ASSERT(gpu_count >= 0, "GPU discovery should complete");
    
    if (gpu_count > 0) {
        // Test first GPU device
        gpu_device_t* gpu = gpu_get_device(0);
        TEST_ASSERT_NOT_NULL(gpu, "First GPU device should be valid");
        
        // Test GPU initialization
        result = gpu_device_init(gpu);
        TEST_ASSERT_EQ(0, result, "GPU device initialization should succeed");
        
        // Test basic GPU operations
        gpu_info_t info;
        result = gpu_get_info(gpu, &info);
        TEST_ASSERT_EQ(0, result, "Getting GPU info should succeed");
        TEST_ASSERT(info.memory_size > 0, "GPU should have memory");
        
        // Test memory allocation
        gpu_memory_handle_t handle = gpu_alloc_memory(gpu, 1024);
        TEST_ASSERT(handle != GPU_INVALID_HANDLE, "GPU memory allocation should succeed");
        
        // Free GPU memory
        gpu_free_memory(gpu, handle);
        
        // Cleanup GPU device
        gpu_device_cleanup(gpu);
    }
    
    return TEST_PASS;
}

DEFINE_TEST(test_audio_driver_integration, "Test audio driver integration with kernel", "integration", true) {
    // Test audio driver loading and initialization
    int result = audio_subsystem_init();
    TEST_ASSERT_EQ(0, result, "Audio subsystem initialization should succeed");
    
    // Discover audio devices
    int audio_count = audio_discover_devices();
    TEST_ASSERT(audio_count >= 0, "Audio discovery should complete");
    
    if (audio_count > 0) {
        // Test first audio device
        audio_device_t* audio = audio_get_device(0);
        TEST_ASSERT_NOT_NULL(audio, "First audio device should be valid");
        
        // Test audio device initialization
        result = audio_device_init(audio);
        TEST_ASSERT_EQ(0, result, "Audio device initialization should succeed");
        
        // Test audio capabilities
        audio_caps_t caps;
        result = audio_get_capabilities(audio, &caps);
        TEST_ASSERT_EQ(0, result, "Getting audio capabilities should succeed");
        TEST_ASSERT(caps.max_sample_rate > 0, "Audio device should support some sample rate");
        
        // Test audio stream creation
        audio_stream_t* stream = audio_create_stream(audio, &caps);
        TEST_ASSERT_NOT_NULL(stream, "Audio stream creation should succeed");
        
        // Test stream operations
        result = audio_start_stream(stream);
        TEST_ASSERT_EQ(0, result, "Starting audio stream should succeed");
        
        result = audio_stop_stream(stream);
        TEST_ASSERT_EQ(0, result, "Stopping audio stream should succeed");
        
        // Cleanup
        audio_destroy_stream(stream);
        audio_device_cleanup(audio);
    }
    
    return TEST_PASS;
}

DEFINE_TEST(test_interrupt_handling_integration, "Test interrupt handling between kernel and drivers", "integration", false) {
    // Test interrupt registration and handling
    int irq_number = 10; // Use a test IRQ number
    
    // Register interrupt handler
    int result = register_interrupt_handler(irq_number, test_interrupt_handler, NULL);
    TEST_ASSERT_EQ(0, result, "Interrupt handler registration should succeed");
    
    // Verify handler is registered
    interrupt_handler_t handler = get_interrupt_handler(irq_number);
    TEST_ASSERT_EQ(test_interrupt_handler, handler, "Registered handler should match");
    
    // Simulate interrupt (in testing environment)
    simulate_interrupt(irq_number);
    
    // Verify interrupt was handled (check global test variable)
    TEST_ASSERT(test_interrupt_handled, "Interrupt should have been handled");
    
    // Unregister handler
    result = unregister_interrupt_handler(irq_number);
    TEST_ASSERT_EQ(0, result, "Interrupt handler unregistration should succeed");
    
    return TEST_PASS;
}

DEFINE_TEST(test_dma_integration, "Test DMA operations between kernel and drivers", "integration", true) {
    // Test DMA buffer allocation and operations
    size_t buffer_size = 4096;
    dma_buffer_t* buffer = dma_alloc_coherent(buffer_size);
    TEST_ASSERT_NOT_NULL(buffer, "DMA buffer allocation should succeed");
    TEST_ASSERT_NOT_NULL(buffer->virtual_addr, "DMA buffer should have virtual address");
    TEST_ASSERT(buffer->physical_addr != 0, "DMA buffer should have physical address");
    TEST_ASSERT_EQ(buffer_size, buffer->size, "DMA buffer size should match requested size");
    
    // Test buffer alignment (should be page-aligned)
    TEST_ASSERT((buffer->physical_addr & 0xFFF) == 0, "DMA buffer should be page-aligned");
    
    // Test buffer access
    uint8_t* data = (uint8_t*)buffer->virtual_addr;
    memset(data, 0xAA, buffer_size);
    TEST_ASSERT(data[0] == 0xAA, "DMA buffer should be writable");
    TEST_ASSERT(data[buffer_size-1] == 0xAA, "DMA buffer should be fully accessible");
    
    // Test DMA mapping for device
    dma_addr_t device_addr = dma_map_single(buffer->virtual_addr, buffer_size, DMA_TO_DEVICE);
    TEST_ASSERT(device_addr != DMA_MAPPING_ERROR, "DMA mapping should succeed");
    
    // Unmap DMA buffer
    dma_unmap_single(device_addr, buffer_size, DMA_TO_DEVICE);
    
    // Free DMA buffer
    dma_free_coherent(buffer);
    
    return TEST_PASS;
}

DEFINE_TEST(test_driver_power_management, "Test driver power management integration", "integration", false) {
    // Test power management callbacks
    driver_info_t power_test_driver = {
        .name = "power_test_driver",
        .version = "1.0.0",
        .type = DRIVER_TYPE_MISC,
        .init = test_driver_init,
        .cleanup = test_driver_cleanup,
        .suspend = test_driver_suspend,
        .resume = test_driver_resume
    };
    
    // Register driver
    int result = register_driver(&power_test_driver);
    TEST_ASSERT_EQ(0, result, "Driver registration should succeed");
    
    // Test suspend operation
    result = driver_suspend_all();
    TEST_ASSERT_EQ(0, result, "Driver suspend should succeed");
    TEST_ASSERT(test_driver_suspended, "Test driver should be suspended");
    
    // Test resume operation
    result = driver_resume_all();
    TEST_ASSERT_EQ(0, result, "Driver resume should succeed");
    TEST_ASSERT(!test_driver_suspended, "Test driver should be resumed");
    
    // Cleanup
    unregister_driver("power_test_driver");
    
    return TEST_PASS;
}

// Mock driver functions for testing
static volatile bool test_interrupt_handled = false;
static volatile bool test_driver_suspended = false;

void test_interrupt_handler(void* data) {
    test_interrupt_handled = true;
}

int test_driver_init(void) {
    return 0;
}

void test_driver_cleanup(void) {
    // Cleanup code
}

int test_driver_probe(void* device) {
    return 0; // Device supported
}

int test_driver_suspend(void) {
    test_driver_suspended = true;
    return 0;
}

int test_driver_resume(void) {
    test_driver_suspended = false;
    return 0;
}

// Test suite setup and teardown
void integration_test_setup(void) {
    // Initialize driver subsystem
    driver_subsystem_init();
    
    // Reset test variables
    test_interrupt_handled = false;
    test_driver_suspended = false;
}

void integration_test_teardown(void) {
    // Cleanup driver subsystem
    driver_subsystem_cleanup();
}

// Register all integration tests
void register_integration_tests(void) {
    test_suite_t* integration_suite = create_test_suite("Kernel-Driver Integration", 
        "Integration tests between kernel and driver subsystems");
    
    integration_suite->setup = integration_test_setup;
    integration_suite->teardown = integration_test_teardown;
    
    REGISTER_TEST(integration_suite, test_driver_registration);
    REGISTER_TEST(integration_suite, test_pci_device_discovery);
    REGISTER_TEST(integration_suite, test_gpu_driver_integration);
    REGISTER_TEST(integration_suite, test_audio_driver_integration);
    REGISTER_TEST(integration_suite, test_interrupt_handling_integration);
    REGISTER_TEST(integration_suite, test_dma_integration);
    REGISTER_TEST(integration_suite, test_driver_power_management);
}
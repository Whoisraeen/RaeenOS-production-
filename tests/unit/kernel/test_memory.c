#include "../test_framework.h"
#include "../../../kernel/memory.h"
#include "../../../kernel/pmm.h"
#include <stdint.h>
#include <string.h>

// Test memory management subsystem

DEFINE_TEST(test_pmm_initialization, "Test physical memory manager initialization", "memory", false) {
    // Test PMM initialization with mock memory map
    uint64_t mock_memory_size = 1024 * 1024; // 1MB for testing
    
    // Initialize PMM (this would normally be done during boot)
    TEST_ASSERT(pmm_init(mock_memory_size) == 0, "PMM initialization should succeed");
    TEST_ASSERT(pmm_get_total_memory() == mock_memory_size, "Total memory should match initialized size");
    TEST_ASSERT(pmm_get_free_memory() > 0, "Free memory should be greater than 0 after init");
    
    return TEST_PASS;
}

DEFINE_TEST(test_pmm_allocation, "Test physical memory allocation and deallocation", "memory", false) {
    // Test basic page allocation
    void* page1 = pmm_alloc_page();
    TEST_ASSERT_NOT_NULL(page1, "First page allocation should succeed");
    
    void* page2 = pmm_alloc_page();
    TEST_ASSERT_NOT_NULL(page2, "Second page allocation should succeed");
    TEST_ASSERT_NEQ((uintptr_t)page1, (uintptr_t)page2, "Allocated pages should have different addresses");
    
    // Test page deallocation
    pmm_free_page(page1);
    pmm_free_page(page2);
    
    // Test multiple page allocation
    void* pages = pmm_alloc_pages(4); // 4 consecutive pages
    TEST_ASSERT_NOT_NULL(pages, "Multi-page allocation should succeed");
    
    pmm_free_pages(pages, 4);
    
    return TEST_PASS;
}

DEFINE_TEST(test_vmm_mapping, "Test virtual memory mapping", "memory", false) {
    // Test virtual memory mapping functionality
    void* virtual_addr = (void*)0x40000000; // Arbitrary virtual address
    void* physical_page = pmm_alloc_page();
    TEST_ASSERT_NOT_NULL(physical_page, "Physical page allocation for mapping should succeed");
    
    // Map virtual address to physical page
    int result = vmm_map_page(virtual_addr, physical_page, VM_READ | VM_WRITE);
    TEST_ASSERT_EQ(0, result, "Virtual memory mapping should succeed");
    
    // Test that the mapping is active
    TEST_ASSERT(vmm_is_mapped(virtual_addr), "Virtual address should be mapped");
    
    // Test unmapping
    vmm_unmap_page(virtual_addr);
    TEST_ASSERT(!vmm_is_mapped(virtual_addr), "Virtual address should be unmapped");
    
    pmm_free_page(physical_page);
    return TEST_PASS;
}

DEFINE_TEST(test_heap_allocation, "Test kernel heap allocation", "memory", false) {
    // Test kernel heap allocator
    void* ptr1 = kmalloc(64);
    TEST_ASSERT_NOT_NULL(ptr1, "Small heap allocation should succeed");
    
    void* ptr2 = kmalloc(1024);
    TEST_ASSERT_NOT_NULL(ptr2, "Medium heap allocation should succeed");
    
    void* ptr3 = kmalloc(4096);
    TEST_ASSERT_NOT_NULL(ptr3, "Large heap allocation should succeed");
    
    // Test memory is writable
    memset(ptr1, 0xAA, 64);
    memset(ptr2, 0xBB, 1024);
    memset(ptr3, 0xCC, 4096);
    
    // Verify memory contents
    TEST_ASSERT(((uint8_t*)ptr1)[0] == 0xAA, "Memory should be writable");
    TEST_ASSERT(((uint8_t*)ptr2)[0] == 0xBB, "Memory should be writable");
    TEST_ASSERT(((uint8_t*)ptr3)[0] == 0xCC, "Memory should be writable");
    
    // Test freeing
    kfree(ptr1);
    kfree(ptr2);
    kfree(ptr3);
    
    return TEST_PASS;
}

DEFINE_TEST(test_memory_protection, "Test memory protection mechanisms", "memory", false) {
    // Test memory protection and segmentation
    void* user_page = pmm_alloc_page();
    void* kernel_page = pmm_alloc_page();
    
    TEST_ASSERT_NOT_NULL(user_page, "User page allocation should succeed");
    TEST_ASSERT_NOT_NULL(kernel_page, "Kernel page allocation should succeed");
    
    // Map pages with different permissions
    void* user_vaddr = (void*)0x80000000;
    void* kernel_vaddr = (void*)0xC0000000;
    
    // User page should be accessible from user mode
    int result1 = vmm_map_page(user_vaddr, user_page, VM_READ | VM_WRITE | VM_USER);
    TEST_ASSERT_EQ(0, result1, "User page mapping should succeed");
    
    // Kernel page should only be accessible from kernel mode
    int result2 = vmm_map_page(kernel_vaddr, kernel_page, VM_READ | VM_WRITE);
    TEST_ASSERT_EQ(0, result2, "Kernel page mapping should succeed");
    
    // Test protection levels
    TEST_ASSERT(vmm_check_access(user_vaddr, VM_USER), "User should have access to user page");
    TEST_ASSERT(!vmm_check_access(kernel_vaddr, VM_USER), "User should not have access to kernel page");
    
    // Cleanup
    vmm_unmap_page(user_vaddr);
    vmm_unmap_page(kernel_vaddr);
    pmm_free_page(user_page);
    pmm_free_page(kernel_page);
    
    return TEST_PASS;
}

DEFINE_TEST(test_memory_stress, "Stress test memory allocation under load", "memory", false) {
    // Stress test with many allocations
    void* ptrs[100];
    int allocated_count = 0;
    
    // Allocate many small blocks
    for (int i = 0; i < 100; i++) {
        ptrs[i] = kmalloc(32 + (i % 64)); // Variable sizes
        if (ptrs[i]) {
            allocated_count++;
            memset(ptrs[i], i & 0xFF, 32 + (i % 64)); // Write pattern
        }
    }
    
    TEST_ASSERT(allocated_count > 90, "Most allocations should succeed under stress");
    
    // Verify patterns
    for (int i = 0; i < allocated_count; i++) {
        if (ptrs[i]) {
            TEST_ASSERT(((uint8_t*)ptrs[i])[0] == (i & 0xFF), "Memory pattern should be preserved");
        }
    }
    
    // Free all allocations
    for (int i = 0; i < allocated_count; i++) {
        if (ptrs[i]) {
            kfree(ptrs[i]);
        }
    }
    
    return TEST_PASS;
}

DEFINE_TEST(test_memory_fragmentation, "Test memory fragmentation handling", "memory", false) {
    // Test allocator's ability to handle fragmentation
    void* large_blocks[10];
    void* small_blocks[20];
    
    // Allocate large blocks
    for (int i = 0; i < 10; i++) {
        large_blocks[i] = kmalloc(4096);
        TEST_ASSERT_NOT_NULL(large_blocks[i], "Large block allocation should succeed");
    }
    
    // Free every other large block to create fragmentation
    for (int i = 1; i < 10; i += 2) {
        kfree(large_blocks[i]);
        large_blocks[i] = NULL;
    }
    
    // Try to allocate small blocks in the gaps
    for (int i = 0; i < 20; i++) {
        small_blocks[i] = kmalloc(1024);
        // Some may fail due to fragmentation, but allocator should handle gracefully
    }
    
    // Cleanup
    for (int i = 0; i < 10; i += 2) {
        if (large_blocks[i]) kfree(large_blocks[i]);
    }
    
    for (int i = 0; i < 20; i++) {
        if (small_blocks[i]) kfree(small_blocks[i]);
    }
    
    return TEST_PASS;
}

// Test suite setup and teardown
void memory_test_setup(void) {
    // Initialize memory management for testing
    // This might involve setting up a mock memory environment
}

void memory_test_teardown(void) {
    // Clean up any test-specific memory allocations
    // Reset memory management to clean state
}

// Register all memory tests
void register_memory_tests(void) {
    test_suite_t* memory_suite = create_test_suite("Memory Management", 
        "Comprehensive tests for RaeenOS memory management subsystem");
    
    memory_suite->setup = memory_test_setup;
    memory_suite->teardown = memory_test_teardown;
    
    REGISTER_TEST(memory_suite, test_pmm_initialization);
    REGISTER_TEST(memory_suite, test_pmm_allocation);
    REGISTER_TEST(memory_suite, test_vmm_mapping);
    REGISTER_TEST(memory_suite, test_heap_allocation);
    REGISTER_TEST(memory_suite, test_memory_protection);
    REGISTER_TEST(memory_suite, test_memory_stress);
    REGISTER_TEST(memory_suite, test_memory_fragmentation);
}
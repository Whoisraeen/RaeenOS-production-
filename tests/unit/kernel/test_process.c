#include "../test_framework.h"
#include "../../../kernel/process/process.h"
#include "../../../kernel/include/syscall.h"
#include <stdint.h>
#include <string.h>

// Test process management subsystem

DEFINE_TEST(test_process_creation, "Test process creation and initialization", "process", false) {
    // Test basic process creation
    process_t* proc = create_process("test_process", PROCESS_USER);
    TEST_ASSERT_NOT_NULL(proc, "Process creation should succeed");
    TEST_ASSERT_EQ(PROCESS_READY, proc->state, "New process should be in READY state");
    TEST_ASSERT_NOT_NULL(proc->memory_space, "Process should have memory space");
    TEST_ASSERT(proc->pid > 0, "Process should have valid PID");
    
    // Test process name
    TEST_ASSERT(strcmp(proc->name, "test_process") == 0, "Process name should match");
    
    // Cleanup
    destroy_process(proc);
    return TEST_PASS;
}

DEFINE_TEST(test_process_scheduling, "Test process scheduling algorithms", "process", false) {
    // Create multiple test processes
    process_t* proc1 = create_process("proc1", PROCESS_USER);
    process_t* proc2 = create_process("proc2", PROCESS_USER);
    process_t* proc3 = create_process("proc3", PROCESS_USER);
    
    TEST_ASSERT_NOT_NULL(proc1, "First process creation should succeed");
    TEST_ASSERT_NOT_NULL(proc2, "Second process creation should succeed");
    TEST_ASSERT_NOT_NULL(proc3, "Third process creation should succeed");
    
    // Set different priorities
    proc1->priority = PRIORITY_HIGH;
    proc2->priority = PRIORITY_NORMAL;
    proc3->priority = PRIORITY_LOW;
    
    // Add to scheduler
    scheduler_add_process(proc1);
    scheduler_add_process(proc2);
    scheduler_add_process(proc3);
    
    // Test scheduling - high priority should run first
    process_t* next = scheduler_get_next_process();
    TEST_ASSERT_EQ(proc1, next, "High priority process should be scheduled first");
    
    // Mark high priority as running, get next
    proc1->state = PROCESS_RUNNING;
    next = scheduler_get_next_process();
    TEST_ASSERT_EQ(proc2, next, "Normal priority should be next");
    
    // Cleanup
    scheduler_remove_process(proc1);
    scheduler_remove_process(proc2);
    scheduler_remove_process(proc3);
    destroy_process(proc1);
    destroy_process(proc2);
    destroy_process(proc3);
    
    return TEST_PASS;
}

DEFINE_TEST(test_context_switching, "Test process context switching", "process", false) {
    // Create two test processes
    process_t* proc1 = create_process("ctx_test1", PROCESS_USER);
    process_t* proc2 = create_process("ctx_test2", PROCESS_USER);
    
    TEST_ASSERT_NOT_NULL(proc1, "First process creation should succeed");
    TEST_ASSERT_NOT_NULL(proc2, "Second process creation should succeed");
    
    // Set up initial register states
    proc1->registers.rax = 0x1111111111111111;
    proc1->registers.rbx = 0x2222222222222222;
    proc2->registers.rax = 0xAAAAAAAAAAAAAAAA;
    proc2->registers.rbx = 0xBBBBBBBBBBBBBBBB;
    
    // Test context switch from proc1 to proc2
    context_switch(proc1, proc2);
    
    // After context switch, current registers should match proc2
    registers_t current_regs = get_current_registers();
    TEST_ASSERT_EQ(0xAAAAAAAAAAAAAAAA, current_regs.rax, "RAX should match proc2 after context switch");
    TEST_ASSERT_EQ(0xBBBBBBBBBBBBBBBB, current_regs.rbx, "RBX should match proc2 after context switch");
    
    // Test switching back
    context_switch(proc2, proc1);
    current_regs = get_current_registers();
    TEST_ASSERT_EQ(0x1111111111111111, current_regs.rax, "RAX should match proc1 after switch back");
    TEST_ASSERT_EQ(0x2222222222222222, current_regs.rbx, "RBX should match proc1 after switch back");
    
    // Cleanup
    destroy_process(proc1);
    destroy_process(proc2);
    
    return TEST_PASS;
}

DEFINE_TEST(test_process_states, "Test process state transitions", "process", false) {
    process_t* proc = create_process("state_test", PROCESS_USER);
    TEST_ASSERT_NOT_NULL(proc, "Process creation should succeed");
    
    // Test initial state
    TEST_ASSERT_EQ(PROCESS_READY, proc->state, "New process should be READY");
    
    // Test state transitions
    set_process_state(proc, PROCESS_RUNNING);
    TEST_ASSERT_EQ(PROCESS_RUNNING, proc->state, "Process should transition to RUNNING");
    
    set_process_state(proc, PROCESS_BLOCKED);
    TEST_ASSERT_EQ(PROCESS_BLOCKED, proc->state, "Process should transition to BLOCKED");
    
    set_process_state(proc, PROCESS_READY);
    TEST_ASSERT_EQ(PROCESS_READY, proc->state, "Process should transition back to READY");
    
    set_process_state(proc, PROCESS_TERMINATED);
    TEST_ASSERT_EQ(PROCESS_TERMINATED, proc->state, "Process should transition to TERMINATED");
    
    // Cleanup
    destroy_process(proc);
    return TEST_PASS;
}

DEFINE_TEST(test_process_memory_isolation, "Test process memory isolation", "process", false) {
    // Create two processes
    process_t* proc1 = create_process("mem_test1", PROCESS_USER);
    process_t* proc2 = create_process("mem_test2", PROCESS_USER);
    
    TEST_ASSERT_NOT_NULL(proc1, "First process creation should succeed");
    TEST_ASSERT_NOT_NULL(proc2, "Second process creation should succeed");
    
    // Allocate memory in each process
    void* mem1 = process_allocate_memory(proc1, 4096);
    void* mem2 = process_allocate_memory(proc2, 4096);
    
    TEST_ASSERT_NOT_NULL(mem1, "Memory allocation in proc1 should succeed");
    TEST_ASSERT_NOT_NULL(mem2, "Memory allocation in proc2 should succeed");
    
    // Virtual addresses might be same, but physical should be different
    TEST_ASSERT_NEQ(get_physical_address(proc1, mem1), 
                   get_physical_address(proc2, mem2), 
                   "Physical addresses should be different");
    
    // Test that one process cannot access another's memory
    bool access_allowed = can_process_access_memory(proc1, mem2);
    TEST_ASSERT(!access_allowed, "Process should not access another process's memory");
    
    // Cleanup
    process_free_memory(proc1, mem1);
    process_free_memory(proc2, mem2);
    destroy_process(proc1);
    destroy_process(proc2);
    
    return TEST_PASS;
}

DEFINE_TEST(test_process_signals, "Test process signal handling", "process", false) {
    process_t* proc = create_process("signal_test", PROCESS_USER);
    TEST_ASSERT_NOT_NULL(proc, "Process creation should succeed");
    
    // Test signal delivery
    int result = send_signal(proc, SIGTERM);
    TEST_ASSERT_EQ(0, result, "Signal delivery should succeed");
    
    // Check that signal is pending
    TEST_ASSERT(has_pending_signal(proc, SIGTERM), "SIGTERM should be pending");
    
    // Test signal handling
    signal_handler_t old_handler = set_signal_handler(proc, SIGTERM, test_signal_handler);
    TEST_ASSERT_NOT_NULL(old_handler, "Setting signal handler should return previous handler");
    
    // Process signal
    int handled = process_pending_signals(proc);
    TEST_ASSERT(handled > 0, "Signal should be processed");
    TEST_ASSERT(!has_pending_signal(proc, SIGTERM), "SIGTERM should no longer be pending");
    
    // Cleanup
    destroy_process(proc);
    return TEST_PASS;
}

DEFINE_TEST(test_process_fork, "Test process forking", "process", false) {
    process_t* parent = create_process("parent", PROCESS_USER);
    TEST_ASSERT_NOT_NULL(parent, "Parent process creation should succeed");
    
    // Set up some state in parent
    parent->registers.rax = 0x12345678;
    void* parent_mem = process_allocate_memory(parent, 4096);
    TEST_ASSERT_NOT_NULL(parent_mem, "Parent memory allocation should succeed");
    
    // Write data to parent memory
    memset(parent_mem, 0xAB, 4096);
    
    // Fork the process
    process_t* child = fork_process(parent);
    TEST_ASSERT_NOT_NULL(child, "Process fork should succeed");
    
    // Child should have different PID
    TEST_ASSERT_NEQ(parent->pid, child->pid, "Child should have different PID");
    
    // Child should inherit register state
    TEST_ASSERT_EQ(parent->registers.rax, child->registers.rax, "Child should inherit register state");
    
    // Child should have copy of memory (copy-on-write)
    void* child_mem = get_memory_mapping(child, parent_mem);
    TEST_ASSERT_NOT_NULL(child_mem, "Child should have memory mapping");
    
    // Verify child has copy of data
    TEST_ASSERT(((uint8_t*)child_mem)[0] == 0xAB, "Child should have copy of parent data");
    
    // Test copy-on-write: modify child memory
    memset(child_mem, 0xCD, 100);
    
    // Parent memory should be unchanged
    TEST_ASSERT(((uint8_t*)parent_mem)[0] == 0xAB, "Parent memory should be unchanged after child modification");
    TEST_ASSERT(((uint8_t*)child_mem)[0] == 0xCD, "Child memory should be modified");
    
    // Cleanup
    process_free_memory(parent, parent_mem);
    destroy_process(parent);
    destroy_process(child);
    
    return TEST_PASS;
}

DEFINE_TEST(test_process_exec, "Test process execution replacement", "process", false) {
    process_t* proc = create_process("exec_test", PROCESS_USER);
    TEST_ASSERT_NOT_NULL(proc, "Process creation should succeed");
    
    // Store original PID
    pid_t original_pid = proc->pid;
    
    // Allocate some memory before exec
    void* old_mem = process_allocate_memory(proc, 4096);
    TEST_ASSERT_NOT_NULL(old_mem, "Memory allocation should succeed");
    
    // Execute new program (mock executable)
    int result = process_exec(proc, "/bin/test_program", NULL, NULL);
    TEST_ASSERT_EQ(0, result, "Process exec should succeed");
    
    // PID should remain the same
    TEST_ASSERT_EQ(original_pid, proc->pid, "PID should remain same after exec");
    
    // Old memory should no longer be accessible
    bool old_mem_valid = can_process_access_memory(proc, old_mem);
    TEST_ASSERT(!old_mem_valid, "Old memory should not be accessible after exec");
    
    // Process should have new memory space
    TEST_ASSERT_NOT_NULL(proc->memory_space, "Process should have new memory space");
    
    // Cleanup
    destroy_process(proc);
    return TEST_PASS;
}

// Mock signal handler for testing
void test_signal_handler(int signal) {
    // Mock signal handler - just mark that it was called
    static volatile int handler_called = 0;
    handler_called = signal;
}

// Test suite setup and teardown
void process_test_setup(void) {
    // Initialize process management system
    process_manager_init();
    scheduler_init();
}

void process_test_teardown(void) {
    // Clean up any remaining test processes
    scheduler_cleanup();
    process_manager_cleanup();
}

// Register all process tests
void register_process_tests(void) {
    test_suite_t* process_suite = create_test_suite("Process Management", 
        "Comprehensive tests for RaeenOS process management subsystem");
    
    process_suite->setup = process_test_setup;
    process_suite->teardown = process_test_teardown;
    
    REGISTER_TEST(process_suite, test_process_creation);
    REGISTER_TEST(process_suite, test_process_scheduling);
    REGISTER_TEST(process_suite, test_context_switching);
    REGISTER_TEST(process_suite, test_process_states);
    REGISTER_TEST(process_suite, test_process_memory_isolation);
    REGISTER_TEST(process_suite, test_process_signals);
    REGISTER_TEST(process_suite, test_process_fork);
    REGISTER_TEST(process_suite, test_process_exec);
}
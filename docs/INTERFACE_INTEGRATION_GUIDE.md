# RaeenOS Interface Integration Guide

**Version:** 1.0  
**Last Updated:** July 30, 2025  
**Target Audience:** All 42 specialized development agents

---

## Overview

This document provides comprehensive guidance for integrating with RaeenOS interfaces to prevent conflicts when multiple agents develop simultaneously. All agents MUST follow these specifications to ensure seamless system integration.

## Critical Integration Points

### 1. Interface Headers Location

All interface headers are located in `/kernel/include/`:

- `hal_interface.h` - Hardware Abstraction Layer
- `driver_framework.h` - Driver Framework APIs
- `memory_interface.h` - Memory Management APIs
- `process_interface.h` - Process & Thread Management
- `filesystem_interface.h` - File System APIs
- `security_interface.h` - Security Framework APIs
- `ai_interface.h` - AI Integration APIs
- `system_services_interface.h` - System Services APIs
- `raeenos_interfaces.h` - Master integration header

### 2. Mandatory Include Order

**ALWAYS include headers in this order:**

```c
#include "types.h"                    // First - basic types
#include "errno.h"                    // Second - error codes
#include "raeenos_interfaces.h"       // Third - master header
// Then include specific interfaces as needed
```

### 3. API Version Compatibility

**Every module MUST check API compatibility:**

```c
RAEENOS_REQUIRE_API_VERSION(1, 0, 0);  // Major, Minor, Patch
```

## Subsystem Dependencies

### Initialization Order (CRITICAL)

Subsystems MUST be initialized in this exact order:

```
1. Early (Order 0-99):
   - HAL (Hardware Abstraction Layer)
   - Memory Management

2. Core (Order 100-199):
   - Process Management
   - Security Framework
   - Driver Framework

3. Services (Order 200-299):
   - File System
   - Network Stack
   - System Services

4. Advanced (Order 300-399):
   - Graphics Subsystem
   - Audio Subsystem
   - AI Integration

5. High-Level (Order 400-499):
   - Application Framework
   - Package Manager

6. Late (Order 500+):
   - User Space Services
```

### Dependency Declaration

Declare dependencies using the provided macro:

```c
RAEENOS_DECLARE_DEPENDENCIES(GRAPHICS, 
    RAEENOS_SUBSYSTEM_MEMORY,
    RAEENOS_SUBSYSTEM_DRIVER_FRAMEWORK,
    RAEENOS_SUBSYSTEM_SECURITY
);
```

## Memory Management Integration

### Required Pattern for All Subsystems

```c
// Use system allocators, not direct malloc/free
void* ptr = RAEENOS_ALLOC(size, MM_FLAG_KERNEL | MM_FLAG_ZERO);
if (!ptr) {
    RAEENOS_ERROR(YOUR_SUBSYSTEM, RAEENOS_ERR_NO_MEMORY, "Allocation failed");
    return -ENOMEM;
}

// Always free allocated memory
RAEENOS_FREE(ptr);
```

### Memory Flags Usage

- `MM_FLAG_KERNEL` - Kernel memory allocation
- `MM_FLAG_USER` - User memory allocation  
- `MM_FLAG_DMA` - DMA-capable memory
- `MM_FLAG_ZERO` - Zero-initialized memory
- `MM_FLAG_ATOMIC` - Atomic allocation (no sleep)

## Process Integration

### Current Process Access

```c
process_t* current = current_process();
thread_t* thread = current_thread();
uint32_t pid = getpid();
uint32_t tid = gettid();
```

### Security Context Validation

```c
if (!SECURITY_CHECK("your_action", "your_resource")) {
    return -EPERM;
}

if (!CAPABILITY_CHECK(CAP_SYS_ADMIN)) {
    return -EPERM;
}
```

## File System Integration

### Standard File Operations

```c
vfs_file_t* file;
int result = vfs_open("/path/to/file", VFS_FLAG_READ, 0644, &file);
if (result != 0) {
    return result;
}

ssize_t bytes_read = vfs_read(file, buffer, sizeof(buffer));
vfs_close(file);
```

### Path Validation

```c
// Always validate paths before use
if (!vfs_path_is_valid(path)) {
    return -EINVAL;
}
```

## Security Integration

### Mandatory Security Checks

```c
// Check permissions before any privileged operation
if (!security_check_permission("operation_name", "resource_path")) {
    RAEENOS_LOG(LOG_WARNING, "subsystem", "Permission denied");
    return -EPERM;
}
```

### Sandbox Application

```c
// Apply sandbox to processes
sandbox_profile_t* profile;
security->create_sandbox("restricted_profile", &profile);
security->apply_sandbox(process, profile);
```

## Driver Framework Integration

### Driver Registration

```c
DEFINE_DRIVER(my_driver, DRIVER_TYPE_NETWORK, device_id_table, &my_ops);

static int __init my_driver_init(void) {
    return driver_register(&my_driver_driver);
}

static void __exit my_driver_exit(void) {
    driver_unregister(&my_driver_driver);
}
```

### Device Operations Implementation

```c
static int my_probe(device_t* dev, const device_id_t* id) {
    // Implement device probing
    return 0;
}

static int my_remove(device_t* dev) {
    // Implement device removal
    return 0;
}

static driver_ops_t my_ops = {
    .probe = my_probe,
    .remove = my_remove,
    // ... other operations
};
```

## AI Integration

### AI Service Access

```c
// Check AI permissions
if (!AI_PERMISSION_CHECK(AI_PERM_INFERENCE)) {
    return -EPERM;
}

// Create AI context
ai_context_t* context;
ai_create_context("default_model", &context);

// Perform inference
char* response;
ai_query(context, "your prompt", &response);

// Cleanup
ai_destroy_context(context);
```

## Error Handling Standards

### Consistent Error Reporting

```c
// Use subsystem-specific error reporting
RAEENOS_ERROR(YOUR_SUBSYSTEM, error_code, "Descriptive error message");

// Return appropriate error codes
return -EINVAL;  // Invalid parameter
return -ENOMEM;  // No memory
return -EPERM;   // Permission denied
return -ENOENT;  // Not found
```

### Error Code Ranges

Each subsystem has a dedicated error code range:

```c
#define YOUR_SUBSYSTEM_ERR_BASE    -XXXX
#define YOUR_SUBSYSTEM_ERR_CUSTOM  (YOUR_SUBSYSTEM_ERR_BASE - 1)
```

## Synchronization Patterns

### Lock Usage

```c
// Create locks through integration points
void* lock = raeenos_get_integration_points()->sys_create_lock();

// Always use RAII pattern
void critical_section(void) {
    raeenos_get_integration_points()->sys_acquire_lock(lock);
    
    // Critical code here
    
    raeenos_get_integration_points()->sys_release_lock(lock);
}
```

### Thread Creation

```c
int create_worker_thread(void) {
    void* thread_handle;
    return raeenos_get_integration_points()->sys_create_thread(
        worker_function, 
        worker_data, 
        &thread_handle
    );
}
```

## Event System Integration

### Event Publishing

```c
// Emit system events
raeenos_get_integration_points()->sys_emit_event(
    "subsystem.event_name", 
    event_data, 
    sizeof(event_data)
);
```

### Event Subscription

```c
// Subscribe to events
raeenos_get_integration_points()->sys_subscribe_event(
    "other_subsystem.event",
    event_handler_function
);
```

## Performance Monitoring

### Operation Tracing

```c
void expensive_operation(void) {
    integration_points_t* ipts = raeenos_get_integration_points();
    
    ipts->sys_performance_start("expensive_operation");
    
    // Your operation here
    
    ipts->sys_performance_end("expensive_operation");
}
```

## Configuration Management

### Reading Configuration

```c
size_t config_size;
char config_value[256];
config_size = sizeof(config_value);

int result = raeenos_get_integration_points()->sys_get_config(
    "subsystem.setting", 
    config_value, 
    &config_size
);
```

### Writing Configuration

```c
const char* new_value = "new_setting_value";
raeenos_get_integration_points()->sys_set_config(
    "subsystem.setting",
    new_value,
    strlen(new_value) + 1
);
```

## Logging Standards

### Consistent Logging

```c
// Use integration point logging
RAEENOS_LOG(LOG_INFO, "subsystem_name", "Informational message");
RAEENOS_LOG(LOG_ERROR, "subsystem_name", "Error occurred");
RAEENOS_LOG(LOG_DEBUG, "subsystem_name", "Debug information");
```

### Log Levels

```c
#define LOG_EMERGENCY   0   // System unusable
#define LOG_ALERT       1   // Action must be taken
#define LOG_CRITICAL    2   // Critical conditions
#define LOG_ERROR       3   // Error conditions
#define LOG_WARNING     4   // Warning conditions
#define LOG_NOTICE      5   // Normal but significant
#define LOG_INFO        6   // Informational messages
#define LOG_DEBUG       7   // Debug messages
```

## Health Monitoring

### Health Check Implementation

```c
int subsystem_health_check(void) {
    // Perform subsystem-specific health checks
    if (critical_component_failed()) {
        return -1;  // Health check failed
    }
    
    return 0;  // Health check passed
}
```

### Statistics Reporting

```c
int subsystem_get_stats(void* stats) {
    subsystem_stats_t* s = (subsystem_stats_t*)stats;
    
    s->uptime = get_subsystem_uptime();
    s->memory_usage = get_memory_usage();
    s->operation_count = get_operation_count();
    
    return 0;
}
```

## Testing Integration

### Unit Test Structure

```c
#include "raeenos_interfaces.h"
#include "test_framework.h"

int test_subsystem_functionality(void) {
    // Setup
    RAEENOS_REQUIRE_API_VERSION(1, 0, 0);
    
    // Test code here
    
    // Cleanup
    return TEST_PASS;
}
```

### Integration Test Pattern

```c
int test_subsystem_integration(void) {
    // Wait for dependencies
    RAEENOS_WAIT_FOR_SUBSYSTEM(MEMORY, 5000);  // 5 second timeout
    RAEENOS_WAIT_FOR_SUBSYSTEM(SECURITY, 5000);
    
    // Test integration
    
    return TEST_PASS;
}
```

## Common Pitfalls to Avoid

### 1. Initialization Order Violations

❌ **DON'T:**
```c
// Calling graphics functions before graphics subsystem is initialized
graphics_create_window();  // WRONG!
```

✅ **DO:**
```c
if (!RAEENOS_CHECK_SUBSYSTEM(GRAPHICS)) {
    return -ENOTREADY;
}
graphics_create_window();  // Correct
```

### 2. Direct Memory Management

❌ **DON'T:**
```c
void* ptr = malloc(size);  // WRONG!
free(ptr);                 // WRONG!
```

✅ **DO:**
```c
void* ptr = RAEENOS_ALLOC(size, MM_FLAG_KERNEL);
RAEENOS_FREE(ptr);
```

### 3. Bypassing Security Checks

❌ **DON'T:**
```c
// Direct file access without permission check
FILE* f = fopen(path, "r");  // WRONG!
```

✅ **DO:**
```c
if (!SECURITY_CHECK("file_read", path)) {
    return -EPERM;
}
vfs_file_t* file;
vfs_open(path, VFS_FLAG_READ, 0, &file);
```

### 4. Missing Error Handling

❌ **DON'T:**
```c
subsystem_operation();  // WRONG! No error checking
```

✅ **DO:**
```c
int result = subsystem_operation();
if (result != 0) {
    RAEENOS_ERROR(SUBSYSTEM, result, "Operation failed");
    return result;
}
```

## Agent Coordination Rules

### 1. Interface Modification Protocol

- **NEVER** modify interface headers without coordination
- Propose changes in the coordination channel first
- Wait for approval from lead architect
- Update API version numbers when making changes

### 2. Dependency Resolution

- Check existing dependencies before adding new ones
- Avoid circular dependencies
- Document all dependencies clearly
- Test dependency resolution during integration

### 3. Resource Sharing

- Use resource reservation system for shared resources
- Release resources promptly after use
- Implement proper cleanup in error paths
- Monitor resource usage for leaks

### 4. Testing Coordination

- Run integration tests before committing
- Coordinate test schedules to avoid conflicts
- Share test results with other agents
- Report integration issues immediately

## Integration Testing Checklist

Before committing any code, verify:

- [ ] All required headers included in correct order
- [ ] API version compatibility checked
- [ ] Dependencies properly declared
- [ ] Memory allocations use RAEENOS_ALLOC/FREE
- [ ] Security checks implemented
- [ ] Error handling follows standards
- [ ] Logging uses integration points
- [ ] Health checks implemented
- [ ] Integration tests pass
- [ ] No circular dependencies introduced

## Emergency Procedures

### System Recovery

If a subsystem fails during integration:

1. Check dependencies are properly initialized
2. Verify API version compatibility
3. Review error logs for clues
4. Perform health check on dependent subsystems
5. Restart subsystem if necessary
6. Report critical failures immediately

### Rollback Protocol

If integration breaks the system:

1. Immediately notify other agents
2. Revert to last known good state
3. Identify root cause
4. Fix issues in isolation
5. Re-test before re-integration

## Support and Resources

### Documentation

- System architecture: `/docs/SYSTEM_ARCHITECTURE.md`
- Agent coordination: `/docs/AGENT_COORDINATION_GUIDE.md`
- Master project plan: `/docs/MASTER_PROJECT_PLAN.md`

### Key Contact Points

- Lead architect decisions on interface changes
- Agent coordination channel for integration issues
- Emergency escalation for critical failures

---

**Remember: Following these integration guidelines is mandatory to prevent system conflicts and ensure successful collaboration among all 42 development agents.**
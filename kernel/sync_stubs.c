// Synchronization Primitives Stubs for RaeenOS
// These are minimal implementations for getting the kernel to link and boot

#include <stdint.h>

// Spinlock structure
typedef struct {
    volatile uint32_t lock;
} spinlock_t;

// Initialize a spinlock
void spinlock_init(spinlock_t* lock) {
    if (lock) {
        lock->lock = 0;
    }
}

// Acquire a spinlock (busy wait)
void spin_lock(spinlock_t* lock) {
    if (!lock) return;
    
    while (__sync_lock_test_and_set(&lock->lock, 1)) {
        // Busy wait with CPU hint
        __asm__ volatile ("pause" ::: "memory");
    }
}

// Release a spinlock
void spin_unlock(spinlock_t* lock) {
    if (!lock) return;
    
    __sync_lock_release(&lock->lock);
}

// Try to acquire a spinlock without blocking
int spin_trylock(spinlock_t* lock) {
    if (!lock) return 0;
    
    return !__sync_lock_test_and_set(&lock->lock, 1);
}
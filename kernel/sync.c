/**
 * @file sync.c
 * @brief Kernel Synchronization Primitives Implementation
 * 
 * This file implements thread-safe synchronization primitives for RaeenOS
 * including spinlocks, mutexes, semaphores, and atomic operations.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/sync.h"
#include "include/types.h"
#include "vga.h"

// Architecture-specific functions (would be in separate arch/ directory)
static inline void arch_spin_lock(volatile uint32_t* lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        cpu_pause();
    }
}

static inline void arch_spin_unlock(volatile uint32_t* lock) {
    __sync_lock_release(lock);
}

static inline bool arch_spin_trylock(volatile uint32_t* lock) {
    return !__sync_lock_test_and_set(lock, 1);
}

static inline uint32_t arch_save_flags_and_cli(void) {
    uint32_t flags;
    __asm__ volatile(
        "pushf\n\t"
        "pop %0\n\t"
        "cli"
        : "=r"(flags)
        :
        : "memory"
    );
    return flags;
}

static inline void arch_restore_flags(uint32_t flags) {
    __asm__ volatile(
        "push %0\n\t"
        "popf"
        :
        : "r"(flags)
        : "memory"
    );
}

// Spinlock operations

void spinlock_init(spinlock_t* lock) {
    if (!lock) return;
    
    lock->locked = 0;
    lock->name = "unnamed";
    lock->cpu_id = 0;
    lock->caller = NULL;
}

void spin_lock(spinlock_t* lock) {
    if (!lock) return;
    
    arch_spin_lock(&lock->locked);
    
    // Debug information (would get CPU ID from current CPU)
    lock->cpu_id = 0; // TODO: get_current_cpu_id();
    lock->caller = __builtin_return_address(0);
}

void spin_unlock(spinlock_t* lock) {
    if (!lock) return;
    
    // Clear debug information
    lock->cpu_id = 0;
    lock->caller = NULL;
    
    arch_spin_unlock(&lock->locked);
}

bool spin_trylock(spinlock_t* lock) {
    if (!lock) return false;
    
    if (arch_spin_trylock(&lock->locked)) {
        lock->cpu_id = 0; // TODO: get_current_cpu_id();
        lock->caller = __builtin_return_address(0);
        return true;
    }
    return false;
}

void spin_lock_irqsave(spinlock_t* lock, uint32_t* flags) {
    if (!lock || !flags) return;
    
    *flags = arch_save_flags_and_cli();
    spin_lock(lock);
}

void spin_unlock_irqrestore(spinlock_t* lock, uint32_t flags) {
    if (!lock) return;
    
    spin_unlock(lock);
    arch_restore_flags(flags);
}

// Mutex operations (simplified implementation)
void mutex_init(mutex_t* mutex) {
    if (!mutex) return;
    
    mutex->locked = 0;
    mutex->owner_tid = 0;
    mutex->recursion_count = 0;
    spinlock_init(&mutex->wait_lock);
    mutex->wait_list = NULL;
    mutex->name = "unnamed";
}

void mutex_lock(mutex_t* mutex) {
    if (!mutex) return;
    
    // Simple implementation - in a real system this would handle
    // thread blocking and scheduling
    uint32_t current_tid = 1; // TODO: get_current_thread_id();
    
    // Handle recursive locking
    if (mutex->owner_tid == current_tid) {
        mutex->recursion_count++;
        return;
    }
    
    // Wait for mutex to become available
    while (!mutex_trylock(mutex)) {
        cpu_pause();
        // In a real implementation, this would block the thread
    }
}

void mutex_unlock(mutex_t* mutex) {
    if (!mutex) return;
    
    uint32_t current_tid = 1; // TODO: get_current_thread_id();
    
    // Check ownership
    if (mutex->owner_tid != current_tid) {
        return; // Error: unlocking mutex not owned by current thread
    }
    
    // Handle recursive unlocking
    if (mutex->recursion_count > 0) {
        mutex->recursion_count--;
        return;
    }
    
    // Release the mutex
    mutex->owner_tid = 0;
    mutex->locked = 0;
    
    // Wake up waiting threads (simplified)
    // In a real implementation, this would wake up blocked threads
}

bool mutex_trylock(mutex_t* mutex) {
    if (!mutex) return false;
    
    uint32_t current_tid = 1; // TODO: get_current_thread_id();
    
    // Handle recursive locking
    if (mutex->owner_tid == current_tid) {
        mutex->recursion_count++;
        return true;
    }
    
    // Try to acquire the mutex
    if (__sync_bool_compare_and_swap(&mutex->locked, 0, 1)) {
        mutex->owner_tid = current_tid;
        return true;
    }
    
    return false;
}

bool mutex_is_locked(mutex_t* mutex) {
    return mutex ? mutex->locked != 0 : false;
}

// Semaphore operations
void semaphore_init(semaphore_t* sem, int32_t count, int32_t max_count) {
    if (!sem) return;
    
    sem->count = count;
    sem->max_count = max_count;
    spinlock_init(&sem->wait_lock);
    sem->wait_list = NULL;
    sem->name = "unnamed";
}

void semaphore_wait(semaphore_t* sem) {
    if (!sem) return;
    
    while (!semaphore_trywait(sem)) {
        cpu_pause();
        // In a real implementation, this would block the thread
    }
}

bool semaphore_trywait(semaphore_t* sem) {
    if (!sem) return false;
    
    spin_lock(&sem->wait_lock);
    
    if (sem->count > 0) {
        sem->count--;
        spin_unlock(&sem->wait_lock);
        return true;
    }
    
    spin_unlock(&sem->wait_lock);
    return false;
}

void semaphore_post(semaphore_t* sem) {
    if (!sem) return;
    
    spin_lock(&sem->wait_lock);
    
    if (sem->count < sem->max_count) {
        sem->count++;
    }
    
    spin_unlock(&sem->wait_lock);
    
    // Wake up waiting threads
    // In a real implementation, this would wake up blocked threads
}

int32_t semaphore_getvalue(semaphore_t* sem) {
    if (!sem) return -1;
    
    spin_lock(&sem->wait_lock);
    int32_t value = sem->count;
    spin_unlock(&sem->wait_lock);
    
    return value;
}

// Read-write lock operations
void rwlock_init(rwlock_t* lock) {
    if (!lock) return;
    
    lock->readers = 0;
    lock->writer = 0;
    spinlock_init(&lock->wait_lock);
    lock->reader_wait_list = NULL;
    lock->writer_wait_list = NULL;
    lock->name = "unnamed";
}

void read_lock(rwlock_t* lock) {
    if (!lock) return;
    
    while (!read_trylock(lock)) {
        cpu_pause();
        // In a real implementation, this would block the thread
    }
}

void read_unlock(rwlock_t* lock) {
    if (!lock) return;
    
    spin_lock(&lock->wait_lock);
    
    if (lock->readers > 0) {
        lock->readers--;
    }
    
    spin_unlock(&lock->wait_lock);
    
    // Wake up waiting writers if no more readers
    // In a real implementation, this would handle thread scheduling
}

void write_lock(rwlock_t* lock) {
    if (!lock) return;
    
    while (!write_trylock(lock)) {
        cpu_pause();
        // In a real implementation, this would block the thread
    }
}

void write_unlock(rwlock_t* lock) {
    if (!lock) return;
    
    spin_lock(&lock->wait_lock);
    lock->writer = 0;
    spin_unlock(&lock->wait_lock);
    
    // Wake up waiting readers/writers
    // In a real implementation, this would handle thread scheduling
}

bool read_trylock(rwlock_t* lock) {
    if (!lock) return false;
    
    spin_lock(&lock->wait_lock);
    
    // Can acquire read lock if no writer
    if (lock->writer == 0) {
        lock->readers++;
        spin_unlock(&lock->wait_lock);
        return true;
    }
    
    spin_unlock(&lock->wait_lock);
    return false;
}

bool write_trylock(rwlock_t* lock) {
    if (!lock) return false;
    
    spin_lock(&lock->wait_lock);
    
    // Can acquire write lock if no readers or writers
    if (lock->readers == 0 && lock->writer == 0) {
        lock->writer = 1;
        spin_unlock(&lock->wait_lock);
        return true;
    }
    
    spin_unlock(&lock->wait_lock);
    return false;
}

// CPU relaxation primitives
void cpu_relax(void) {
    __asm__ volatile("pause" ::: "memory");
}

void cpu_pause(void) {
    __asm__ volatile("pause" ::: "memory");
}

// Simple red-black tree operations (stub implementation)
void rb_init_node(struct rb_node* node) {
    if (!node) return;
    
    node->__rb_parent_color = 0;
    node->rb_left = NULL;
    node->rb_right = NULL;
}

void rb_insert_node(struct rb_root* root, struct rb_node* node,
                    int (*compare)(struct rb_node*, struct rb_node*)) {
    // Stub implementation - a real RB tree would be more complex
    if (!root || !node) return;
    
    if (!root->rb_node) {
        root->rb_node = node;
    }
}

void rb_erase_node(struct rb_root* root, struct rb_node* node) {
    // Stub implementation
    if (!root || !node) return;
    
    if (root->rb_node == node) {
        root->rb_node = NULL;
    }
}

struct rb_node* rb_first(struct rb_root* root) {
    return root ? root->rb_node : NULL;
}

struct rb_node* rb_last(struct rb_root* root) {
    return root ? root->rb_node : NULL;
}

struct rb_node* rb_next(struct rb_node* node) {
    return NULL; // Stub
}

struct rb_node* rb_prev(struct rb_node* node) {
    return NULL; // Stub
}
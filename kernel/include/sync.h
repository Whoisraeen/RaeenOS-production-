#ifndef SYNC_H
#define SYNC_H

/**
 * @file sync.h
 * @brief Kernel Synchronization Primitives for RaeenOS
 * 
 * This header provides thread-safe synchronization primitives including
 * spinlocks, mutexes, semaphores, and atomic operations.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "types.h"
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct spinlock spinlock_t;
typedef struct mutex mutex_t;
typedef struct semaphore semaphore_t;
typedef struct rwlock rwlock_t;

// Atomic types
typedef _Atomic int atomic_t;
typedef _Atomic long atomic64_t;

// Spinlock structure
struct spinlock {
    volatile uint32_t locked;
    const char* name;
    uint32_t cpu_id;
    void* caller;
} __attribute__((aligned(4)));

// Mutex structure
struct mutex {
    volatile uint32_t locked;
    uint32_t owner_tid;
    uint32_t recursion_count;
    spinlock_t wait_lock;
    void* wait_list;
    const char* name;
};

// Semaphore structure
struct semaphore {
    volatile int32_t count;
    int32_t max_count;
    spinlock_t wait_lock;
    void* wait_list;
    const char* name;
};

// Read-write lock structure
struct rwlock {
    volatile uint32_t readers;
    volatile uint32_t writer;
    spinlock_t wait_lock;
    void* reader_wait_list;
    void* writer_wait_list;
    const char* name;
};

// List structures for kernel data structures
struct list_head {
    struct list_head* next;
    struct list_head* prev;
};

// Red-black tree node
struct rb_node {
    unsigned long __rb_parent_color;
    struct rb_node* rb_right;
    struct rb_node* rb_left;
};

struct rb_root {
    struct rb_node* rb_node;
};

// Spinlock operations
void spinlock_init(spinlock_t* lock);
void spin_lock(spinlock_t* lock);
void spin_unlock(spinlock_t* lock);
bool spin_trylock(spinlock_t* lock);
void spin_lock_irqsave(spinlock_t* lock, uint32_t* flags);
void spin_unlock_irqrestore(spinlock_t* lock, uint32_t flags);

// Mutex operations
void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);
bool mutex_trylock(mutex_t* mutex);
bool mutex_is_locked(mutex_t* mutex);

// Semaphore operations
void semaphore_init(semaphore_t* sem, int32_t count, int32_t max_count);
void semaphore_wait(semaphore_t* sem);
bool semaphore_trywait(semaphore_t* sem);
void semaphore_post(semaphore_t* sem);
int32_t semaphore_getvalue(semaphore_t* sem);

// Read-write lock operations
void rwlock_init(rwlock_t* lock);
void read_lock(rwlock_t* lock);
void read_unlock(rwlock_t* lock);
void write_lock(rwlock_t* lock);
void write_unlock(rwlock_t* lock);
bool read_trylock(rwlock_t* lock);
bool write_trylock(rwlock_t* lock);

// Atomic operations
static inline int atomic_read(const atomic_t* v) {
    return atomic_load(v);
}

static inline void atomic_set(atomic_t* v, int i) {
    atomic_store(v, i);
}

static inline void atomic_inc(atomic_t* v) {
    atomic_fetch_add(v, 1);
}

static inline void atomic_dec(atomic_t* v) {
    atomic_fetch_sub(v, 1);
}

static inline int atomic_inc_return(atomic_t* v) {
    return atomic_fetch_add(v, 1) + 1;
}

static inline int atomic_dec_return(atomic_t* v) {
    return atomic_fetch_sub(v, 1) - 1;
}

static inline bool atomic_dec_and_test(atomic_t* v) {
    return atomic_fetch_sub(v, 1) == 1;
}

static inline bool atomic_inc_and_test(atomic_t* v) {
    return atomic_fetch_add(v, 1) == -1;
}

static inline int atomic_add_return(atomic_t* v, int i) {
    return atomic_fetch_add(v, i) + i;
}

static inline int atomic_sub_return(atomic_t* v, int i) {
    return atomic_fetch_sub(v, i) - i;
}

static inline bool atomic_cmpxchg(atomic_t* v, int old, int new) {
    return atomic_compare_exchange_strong(v, &old, new);
}

// 64-bit atomic operations
static inline long atomic64_read(const atomic64_t* v) {
    return atomic_load(v);
}

static inline void atomic64_set(atomic64_t* v, long i) {
    atomic_store(v, i);
}

static inline void atomic64_inc(atomic64_t* v) {
    atomic_fetch_add(v, 1);
}

static inline void atomic64_dec(atomic64_t* v) {
    atomic_fetch_sub(v, 1);
}

// Bit operations
static inline void atomic_set_bit(int nr, atomic_t* addr) {
    atomic_fetch_or(addr, (1U << nr));
}

static inline void atomic_clear_bit(int nr, atomic_t* addr) {
    atomic_fetch_and(addr, ~(1U << nr));
}

static inline bool atomic_test_bit(int nr, const atomic_t* addr) {
    return (atomic_read(addr) & (1U << nr)) != 0;
}

static inline bool atomic_test_and_set_bit(int nr, atomic_t* addr) {
    uint32_t old = atomic_fetch_or(addr, (1U << nr));
    return (old & (1U << nr)) != 0;
}

static inline bool atomic_test_and_clear_bit(int nr, atomic_t* addr) {
    uint32_t old = atomic_fetch_and(addr, ~(1U << nr));
    return (old & (1U << nr)) != 0;
}

// List operations
static inline void INIT_LIST_HEAD(struct list_head* list) {
    list->next = list;
    list->prev = list;
}

static inline void list_add(struct list_head* new_entry, struct list_head* head) {
    new_entry->next = head->next;
    new_entry->prev = head;
    head->next->prev = new_entry;
    head->next = new_entry;
}

static inline void list_del(struct list_head* entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->next = entry;
    entry->prev = entry;
}

static inline bool list_empty(const struct list_head* head) {
    return head->next == head;
}

#define list_entry(ptr, type, member) \
    ((type*)((char*)(ptr) - (unsigned long)(&((type*)0)->member)))

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

// Memory barriers
static inline void memory_barrier(void) {
    atomic_thread_fence(memory_order_seq_cst);
}

static inline void read_barrier(void) {
    atomic_thread_fence(memory_order_acquire);
}

static inline void write_barrier(void) {
    atomic_thread_fence(memory_order_release);
}

// CPU-specific barriers
void cpu_relax(void);
void cpu_pause(void);

// Red-black tree operations
void rb_init_node(struct rb_node* node);
void rb_insert_node(struct rb_root* root, struct rb_node* node, 
                    int (*compare)(struct rb_node*, struct rb_node*));
void rb_erase_node(struct rb_root* root, struct rb_node* node);
struct rb_node* rb_first(struct rb_root* root);
struct rb_node* rb_last(struct rb_root* root);
struct rb_node* rb_next(struct rb_node* node);
struct rb_node* rb_prev(struct rb_node* node);

#ifdef __cplusplus
}
#endif

#endif // SYNC_H
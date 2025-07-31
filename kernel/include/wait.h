#ifndef WAIT_H
#define WAIT_H

// Forward declaration to avoid circular dependency
typedef struct process process_t;

// A wait queue is a list of processes waiting for an event.
typedef struct wait_queue {
    process_t* process;
    struct wait_queue* next;
} wait_queue_t;

// Initialize a wait queue.
void wait_queue_init(wait_queue_t** queue);

// Add a process to a wait queue.
void wait_queue_add(wait_queue_t** queue, process_t* process);

// Remove a process from a wait queue.
void wait_queue_remove(wait_queue_t** queue, process_t* process);

// Wake up all processes in a wait queue.
void wait_queue_wake_all(wait_queue_t** queue);

// Wake up a single process from a wait queue.
void wait_queue_wake_one(wait_queue_t** queue);

#endif // WAIT_H

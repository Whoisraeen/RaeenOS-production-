#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include "../include/wait.h"

// IPC Semaphore structure
typedef struct {
    int32_t count;
    wait_queue_t* wait_queue;
} ipc_semaphore_t;

// Initialize an IPC semaphore
void ipc_semaphore_init(ipc_semaphore_t* sem, int32_t initial_count);

// Wait (decrement) an IPC semaphore
void ipc_semaphore_wait(ipc_semaphore_t* sem);

// Signal (increment) an IPC semaphore
void ipc_semaphore_signal(ipc_semaphore_t* sem);

#endif // SEMAPHORE_H

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include "../include/wait.h"

// Semaphore structure
typedef struct {
    int32_t count;
    wait_queue_t* wait_queue;
} semaphore_t;

// Initialize a semaphore
void semaphore_init(semaphore_t* sem, int32_t initial_count);

// Wait (decrement) a semaphore
void semaphore_wait(semaphore_t* sem);

// Signal (increment) a semaphore
void semaphore_signal(semaphore_t* sem);

#endif // SEMAPHORE_H

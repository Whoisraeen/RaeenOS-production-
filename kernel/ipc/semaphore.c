#include "semaphore.h"
#include "../../kernel/process/process.h"
#include "../../kernel/vga.h"

void semaphore_init(semaphore_t* sem, int32_t initial_count) {
    sem->count = initial_count;
    wait_queue_init(&sem->wait_queue);
}

void semaphore_wait(semaphore_t* sem) {
    // Disable interrupts to prevent race conditions
    asm volatile("cli");

    sem->count--;
    if (sem->count < 0) {
        // Block the current process
        wait_queue_add(&sem->wait_queue, get_current_process());
        get_current_process()->state = PROCESS_STATE_SLEEPING;
        asm volatile("sti"); // Re-enable interrupts before scheduling
        schedule();
    }
    asm volatile("sti"); // Re-enable interrupts
}

void semaphore_signal(semaphore_t* sem) {
    // Disable interrupts to prevent race conditions
    asm volatile("cli");

    sem->count++;
    if (sem->count <= 0) {
        // Wake up a waiting process
        wait_queue_wake_one(&sem->wait_queue);
    }
    asm volatile("sti"); // Re-enable interrupts
}

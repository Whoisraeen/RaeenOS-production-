#include "../include/wait.h"
#include "../memory.h"

void wait_queue_init(wait_queue_t** queue) {
    *queue = NULL;
}

void wait_queue_add(wait_queue_t** queue, process_t* process) {
    wait_queue_t* new_entry = (wait_queue_t*)kmalloc(sizeof(wait_queue_t));
    if (!new_entry) {
        // PANIC! Out of memory.
        return;
    }

    new_entry->process = process;
    new_entry->next = *queue;
    *queue = new_entry;
}

void wait_queue_remove(wait_queue_t** queue, process_t* process) {
    wait_queue_t* current = *queue;
    wait_queue_t* prev = NULL;

    while (current) {
        if (current->process == process) {
            if (prev) {
                prev->next = current->next;
            } else {
                *queue = current->next;
            }
            kfree(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void wait_queue_wake_all(wait_queue_t** queue) {
    wait_queue_t* current = *queue;
    while (current) {
        current->process->state = PROCESS_STATE_READY;
        current = current->next;
    }
}

void wait_queue_wake_one(wait_queue_t** queue) {
    if (*queue) {
        wait_queue_t* woken_entry = *queue;
        woken_entry->process->state = PROCESS_STATE_READY;
        *queue = woken_entry->next;
        kfree(woken_entry);
    }
}

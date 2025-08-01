// RaeenOS Pipe IPC Implementation - Source

#include "pipe.h"
#include "../pmm_production.h"
#include "../process/process.h"
#include "../string.h"

pipe_t* pipe_create(void) {
    size_t size = PIPE_DEFAULT_SIZE;
    pipe_t* pipe = (pipe_t*)pmm_alloc_page(GFP_KERNEL, -1);
    if (!pipe) {
        return NULL;
    }

    pipe->buffer = (uint8_t*)pmm_alloc_pages((size + PAGE_SIZE - 1) / PAGE_SIZE, GFP_KERNEL, -1);
    if (!pipe->buffer) {
        pmm_free_page(pipe);
        return NULL;
    }

    pipe->size = size;
    pipe->read_pos = 0;
    pipe->write_pos = 0;
    pipe->count = 0;
    wait_queue_init(&pipe->read_queue);
    wait_queue_init(&pipe->write_queue);

    return pipe;
}

int pipe_read(pipe_t* pipe, uint8_t* buf, int count) {
    if (!pipe || !buf) return -1; // Error

    while (pipe->count == 0) {
        // Pipe is empty, wait for data
        wait_queue_add(&pipe->read_queue, get_current_process());
        get_current_process()->state = PROCESS_STATE_SLEEPING;
        schedule();
    }

    int bytes_read = 0;
    while (bytes_read < count && pipe->count > 0) {
        buf[bytes_read] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % pipe->size;
        pipe->count--;
        bytes_read++;
    }

    // Wake up any waiting writers
    wait_queue_wake_all(&pipe->write_queue);

    return bytes_read;
}

int pipe_write(pipe_t* pipe, const uint8_t* buf, int count) {
    if (!pipe || !buf) return -1; // Error

    while (pipe->count == pipe->size) {
        // Pipe is full, wait for space
        wait_queue_add(&pipe->write_queue, get_current_process());
        get_current_process()->state = PROCESS_STATE_SLEEPING;
        schedule();
    }

    int bytes_written = 0;
    while (bytes_written < count && pipe->count < pipe->size) {
        pipe->buffer[pipe->write_pos] = buf[bytes_written];
        pipe->write_pos = (pipe->write_pos + 1) % pipe->size;
        pipe->count++;
        bytes_written++;
    }

    // Wake up any waiting readers
    wait_queue_wake_all(&pipe->read_queue);

    return bytes_written;
}

void pipe_destroy(pipe_t* pipe) {
    if (!pipe) return;
    if (pipe->buffer) {
        pmm_free_pages(pipe->buffer, (pipe->size + PAGE_SIZE - 1) / PAGE_SIZE);
    }
    pmm_free_page(pipe);
}

#include "message_queue.h"
#include "../memory.h"
#include "../libs/libc/include/string.h"
#include "../vga.h"

static uint32_t next_msg_queue_id = 0;

message_queue_t* msg_queue_create(const char* name) {
    message_queue_t* queue = (message_queue_t*)kmalloc(sizeof(message_queue_t));
    if (!queue) {
        return NULL;
    }

    memset(queue, 0, sizeof(message_queue_t));
    queue->id = next_msg_queue_id++;
    strncpy(queue->name, name, sizeof(queue->name) - 1);
    spinlock_init(&queue->lock);

    debug_print("Message queue created: ");
    debug_print(queue->name);
    debug_print(" (ID: ");
    vga_put_hex(queue->id);
    debug_print(")\n");

    return queue;
}

void msg_queue_destroy(message_queue_t* queue) {
    if (!queue) return;

    spinlock_acquire(&queue->lock);
    debug_print("Message queue destroyed: ");
    debug_print(queue->name);
    debug_print(" (ID: ");
    vga_put_hex(queue->id);
    debug_print(")\n");
    spinlock_release(&queue->lock);

    kfree(queue);
}

int msg_queue_send(message_queue_t* queue, const message_t* msg) {
    if (!queue || !msg) return -1;

    spinlock_acquire(&queue->lock);

    if (queue->count >= MAX_MESSAGES_IN_QUEUE) {
        debug_print("Message queue full: ");
        debug_print(queue->name);
        debug_print("\n");
        spinlock_release(&queue->lock);
        return -1; // Queue full
    }

    memcpy(&queue->messages[queue->tail], msg, sizeof(message_t));
    queue->tail = (queue->tail + 1) % MAX_MESSAGES_IN_QUEUE;
    queue->count++;

    debug_print("Message sent to queue: ");
    debug_print(queue->name);
    debug_print(" (count: ");
    vga_put_hex(queue->count);
    debug_print(")\n");

    spinlock_release(&queue->lock);
    return 0;
}

int msg_queue_receive(message_queue_t* queue, message_t* msg) {
    if (!queue || !msg) return -1;

    spinlock_acquire(&queue->lock);

    if (queue->count == 0) {
        debug_print("Message queue empty: ");
        debug_print(queue->name);
        debug_print("\n");
        spinlock_release(&queue->lock);
        return -1; // Queue empty
    }

    memcpy(msg, &queue->messages[queue->head], sizeof(message_t));
    queue->head = (queue->head + 1) % MAX_MESSAGES_IN_QUEUE;
    queue->count--;

    debug_print("Message received from queue: ");
    debug_print(queue->name);
    debug_print(" (count: ");
    vga_put_hex(queue->count);
    debug_print(")\n");

    spinlock_release(&queue->lock);
    return 0;
}

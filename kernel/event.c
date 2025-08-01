#include "include/event.h"
// Using types.h for kernel build
#include "memory.h"

#define EVENT_QUEUE_SIZE 128

static event_t event_queue[EVENT_QUEUE_SIZE];
static uint32_t head = 0;
static uint32_t tail = 0;
static uint32_t count = 0;

void event_queue_init(void) {
    head = 0;
    tail = 0;
    count = 0;
}

bool event_queue_push(event_t event) {
    if (count == EVENT_QUEUE_SIZE) {
        return false; // Queue is full
    }

    event_queue[tail] = event;
    tail = (tail + 1) % EVENT_QUEUE_SIZE;
    count++;
    return true;
}

bool event_queue_pop(event_t* event) {
    if (count == 0) {
        return false; // Queue is empty
    }

    *event = event_queue[head];
    head = (head + 1) % EVENT_QUEUE_SIZE;
    count--;
    return true;
}

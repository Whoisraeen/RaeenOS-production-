#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stdint.h>
#include <stddef.h>
#include "../sync.h"

#define MAX_MESSAGE_SIZE 256
#define MAX_MESSAGES_IN_QUEUE 16

typedef struct message {
    uint32_t type;
    size_t size;
    uint8_t data[MAX_MESSAGE_SIZE];
} message_t;

typedef struct message_queue {
    uint32_t id;
    char name[64];
    message_t messages[MAX_MESSAGES_IN_QUEUE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    spinlock_t lock;
    // Add wait queues for senders/receivers
} message_queue_t;

// Function to create a new message queue
message_queue_t* msg_queue_create(const char* name);

// Function to destroy a message queue
void msg_queue_destroy(message_queue_t* queue);

// Function to send a message to a queue
int msg_queue_send(message_queue_t* queue, const message_t* msg);

// Function to receive a message from a queue
int msg_queue_receive(message_queue_t* queue, message_t* msg);

#endif // MESSAGE_QUEUE_H

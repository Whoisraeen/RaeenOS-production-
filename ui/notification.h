#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <stdint.h>

// Notification structure
typedef struct {
    char title[64];
    char message[256];
    uint32_t timeout_ms; // 0 for persistent
    // Add icon, actions, etc.
} notification_t;

// Initialize the notification system
void notification_init(void);

// Show a notification
void notification_show(notification_t* notification);

#endif // NOTIFICATION_H

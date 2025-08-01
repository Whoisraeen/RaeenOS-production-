#ifndef SSH_H
#define SSH_H

#include <stdint.h>

// SSH client functions (placeholders)
int ssh_connect(const char* host, uint16_t port, const char* username, const char* password);
int ssh_execute_command(int session_id, const char* command);
int ssh_disconnect(int session_id);

#endif // SSH_H

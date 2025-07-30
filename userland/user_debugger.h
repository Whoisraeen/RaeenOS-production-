#ifndef USER_DEBUGGER_H
#define USER_DEBUGGER_H

#include <stdint.h>

// Initialize the user-mode debugger
void user_debugger_init(void);

// Attach to a user process
int user_debugger_attach(uint32_t pid);

// Detach from a user process
void user_debugger_detach(void);

// Set a breakpoint in user code
int user_debugger_set_breakpoint(uint32_t address);

// Continue user process execution
void user_debugger_continue(void);

#endif // USER_DEBUGGER_H

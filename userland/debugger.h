#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

// Initialize the debugger
void debugger_init(void);

// Attach to a process
int debugger_attach(uint32_t pid);

// Detach from a process
void debugger_detach(void);

// Set a breakpoint
int debugger_set_breakpoint(uint32_t address);

// Continue execution
void debugger_continue(void);

#endif // DEBUGGER_H

#ifndef KERNEL_DEBUGGER_H
#define KERNEL_DEBUGGER_H

#include "include/types.h"

// Initialize the kernel debugger
void kernel_debugger_init(void);

// Enter the kernel debugger
void kernel_debugger_enter(void);

// Print a message to the kernel debugger console
void kernel_debugger_print(const char* message);

#endif // KERNEL_DEBUGGER_H

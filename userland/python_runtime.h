#ifndef PYTHON_RUNTIME_H
#define PYTHON_RUNTIME_H

#include <stdint.h>

// Initialize the Python runtime
void python_runtime_init(void);

// Execute a Python script
int python_runtime_exec_script(const char* script_path);

#endif // PYTHON_RUNTIME_H

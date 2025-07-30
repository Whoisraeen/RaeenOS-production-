#ifndef PROFILER_H
#define PROFILER_H

#include <stdint.h>

// Initialize the profiler
void profiler_init(void);

// Start profiling a function/code block
void profiler_start(const char* name);

// Stop profiling and record results
void profiler_stop(void);

// Get profiling report
void profiler_get_report(void);

#endif // PROFILER_H

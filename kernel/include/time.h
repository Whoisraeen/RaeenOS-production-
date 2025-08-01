#ifndef TIME_H
#define TIME_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#include "types.h"
#endif

// Structure for timeval (seconds and microseconds)
typedef struct {
    long tv_sec;  // seconds
    long tv_usec; // microseconds
} timeval;

// Structure for timezone (minutes west of GMT and DST corrections)
typedef struct {
    int tz_minuteswest; // minutes west of GMT
    int tz_dsttime;     // type of DST correction
} timezone;

#endif // TIME_H

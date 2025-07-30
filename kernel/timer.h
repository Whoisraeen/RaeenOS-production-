// RaeenOS Programmable Interval Timer (PIT) Driver

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Initializes the PIT and registers the timer interrupt handler.
// frequency: The desired frequency of the timer interrupt in Hz.
void timer_init(uint32_t frequency);

#endif // TIMER_H

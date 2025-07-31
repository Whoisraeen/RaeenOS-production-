// RaeenOS Programmable Interval Timer (PIT) Driver

#include "timer.h"
#include "idt.h"
#include "ports.h"
#include "process/process.h"

volatile uint32_t timer_ticks = 0;

// The handler for the timer interrupt.
void timer_handler(registers_t* regs) {
    timer_ticks++;
    schedule(); // Call the scheduler this to switch tasks for multitasking.
}

// Initializes the PIT and registers the timer interrupt handler.
void timer_init(uint32_t frequency) {
    // The PIT's input frequency is approximately 1.193182 MHz.
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte to the command port (0x43).
    // 0x36 sets Channel 0, LSB/MSB access, and Mode 2 (rate generator).
    outb(0x43, 0x36);

    // Send the divisor value, low byte then high byte.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);

    // Register the timer handler for IRQ0 (interrupt 32).
    register_interrupt_handler(32, timer_handler);
}

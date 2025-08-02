// RaeenOS Programmable Interrupt Controller (PIC) Driver

#include "pic.h"
#include "ports.h"

// PIC I/O ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// End-of-Interrupt command
#define PIC_EOI      0x20

// Initialization Command Words
#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

// Remaps the PIC to a new offset.
void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    a1 = inb(PIC1_DATA); // save masks
    a2 = inb(PIC2_DATA);

    // starts the initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // ICW2: master PIC vector offset
    outb(PIC1_DATA, offset1);
    // ICW2: slave PIC vector offset
    outb(PIC2_DATA, offset2);

    // ICW3: tell master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 4);
    // ICW3: tell slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);

    // ICW4: have the PICs use 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // restore saved masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

// Initialize the PIC (remap to standard offsets)
void pic_init(void) {
    // Remap PIC interrupts from 32-47 (standard kernel setup)
    pic_remap(32, 40);
}

// Sends an End-of-Interrupt (EOI) signal to the PICs.
void pic_send_eoi(unsigned char irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

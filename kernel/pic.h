// RaeenOS Programmable Interrupt Controller (PIC) Driver

#ifndef PIC_H
#define PIC_H

// Remaps the PIC to a new offset.
// The master PIC will be at offset1, and the slave at offset2.
void pic_remap(int offset1, int offset2);

// Sends an End-of-Interrupt (EOI) signal to the PIC(s).
void pic_send_eoi(unsigned char irq);

#endif // PIC_H

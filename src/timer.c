/*
 * @file timer.c
 * @version 0.0.1
 * PIT (Programmable Interval Timer) for cursor blinking
 */

#include "timer.h"
#include "cursor.h"

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void timer_init(void) {
    // PIT frequency: 1193180 Hz
    // Desired frequency: ~18.2 Hz (like BIOS timer)
    // Divisor = 1193180 / 18.2 ≈ 65536 (0x10000, wraps to 0)

    // Command byte: channel 0, mode 3 (square wave), binary
    outb(0x43, 0x36);

    // Send divisor (0 = 65536)
    outb(0x40, 0x00);
    outb(0x40, 0x00);
}

void timer_handler(void) {
    // Update cursor blink state
    cursor_tick();
}

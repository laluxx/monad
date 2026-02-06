/*
 * @file keyboard.c
 * @version 0.0.3
 * Keyboard driver
 */

#include "keyboard.h"
#include "cursor.h"
#include "timer.h"

// Port I/O
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Keyboard state
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;

// Input buffer
#define BUFFER_SIZE 256
static char input_buffer[BUFFER_SIZE];
static uint32_t buffer_head = 0;
static uint32_t buffer_tail = 0;

// Initialize IDT
void idt_init(struct idt_entry* idt, struct idt_ptr* idtp) {
    idtp->limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp->base = (uint32_t)idt;

    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt[i].base_low = 0;
        idt[i].base_high = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].flags = 0;
    }

    // Set timer interrupt (IRQ0 = INT 0x20)
    uint32_t timer_addr = (uint32_t)irq0_handler;
    idt[0x20].base_low = timer_addr & 0xFFFF;
    idt[0x20].base_high = (timer_addr >> 16) & 0xFFFF;
    idt[0x20].selector = 0x08;
    idt[0x20].zero = 0;
    idt[0x20].flags = 0x8E;

    // Set keyboard interrupt (IRQ1 = INT 0x21)
    uint32_t kb_addr = (uint32_t)irq1_handler;
    idt[0x21].base_low = kb_addr & 0xFFFF;
    idt[0x21].base_high = (kb_addr >> 16) & 0xFFFF;
    idt[0x21].selector = 0x08;
    idt[0x21].zero = 0;
    idt[0x21].flags = 0x8E;

    // Load IDT
    idt_load(idtp);
}

// Initialize PIC
void pic_init(void) {
    // ICW1: Initialize PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2: Remap IRQs to 0x20-0x2F
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3: Tell PICs about each other
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4: 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Unmask IRQ0 (timer) and IRQ1 (keyboard)
    outb(0x21, 0xFC);  // 11111100 - enable IRQ0 and IRQ1
    outb(0xA1, 0xFF);
}

// Keyboard interrupt handler
void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    // Reset cursor blink on any key event
    cursor_reset_blink();

    // Handle Ctrl press
    if (scancode == SCANCODE_LCTRL) {
        ctrl_pressed = 1;
        return;
    }

    // Handle Ctrl release
    if (scancode == SCANCODE_LCTRL_REL) {
        ctrl_pressed = 0;
        return;
    }

    // Handle shift press
    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
        shift_pressed = 1;
        return;
    }

    // Handle shift release
    if (scancode == SCANCODE_LSHIFT_REL || scancode == SCANCODE_RSHIFT_REL) {
        shift_pressed = 0;
        return;
    }

    // Only handle key press (scancode < 0x80)
    if (scancode < 0x80) {
        char c = shift_pressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
        if (c) {
            // If Ctrl is pressed, convert to control character
            if (ctrl_pressed && c >= 'a' && c <= 'z') {
                c = c - 'a' + 1;
            }

            // Add to buffer
            uint32_t next_head = (buffer_head + 1) % BUFFER_SIZE;
            if (next_head != buffer_tail) {
                input_buffer[buffer_head] = c;
                buffer_head = next_head;
            }
        }
    }
}

// Check if input is available
uint8_t keyboard_has_input(void) {
    return buffer_head != buffer_tail;
}

// Get character from buffer
char keyboard_getchar(void) {
    if (buffer_head == buffer_tail) {
        return 0;
    }

    char c = input_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;

    return c;
}

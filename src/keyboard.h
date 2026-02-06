/*
 * @file keyboard.h
 * @version 0.0.3
 * Keyboard driver and interrupt structures
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// IDT entry structure
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Control keys
#define KEY_CTRL_A  1
#define KEY_CTRL_B  2
#define KEY_CTRL_E  5
#define KEY_CTRL_F  6
#define KEY_CTRL_K  11
#define KEY_CTRL_D  4

// External assembly functions
extern void idt_load(struct idt_ptr* idt_ptr);
extern void irq0_handler(void);  // Timer
extern void irq1_handler(void);  // Keyboard

// Function declarations
void idt_init(struct idt_entry* idt, struct idt_ptr* idtp);
void pic_init(void);
void keyboard_handler(void);
uint8_t keyboard_has_input(void);
char keyboard_getchar(void);

// Scancode definitions
#define SCANCODE_LSHIFT     0x2A
#define SCANCODE_RSHIFT     0x36
#define SCANCODE_LSHIFT_REL 0xAA
#define SCANCODE_RSHIFT_REL 0xB6
#define SCANCODE_LCTRL      0x1D
#define SCANCODE_RCTRL      0x1D
#define SCANCODE_LCTRL_REL  0x9D
#define SCANCODE_RCTRL_REL  0x9D

// Scancode to ASCII tables
static const char scancode_to_ascii[128] = {
    0,    27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
   '9',  '0',  '-',  '=',  '\b', '\t', 'q',  'w',  'e',  'r',
   't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n',  0,
   'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
   '\'', '`',   0,   '\\', 'z',  'x',  'c',  'v',  'b',  'n',
   'm',  ',',  '.',  '/',   0,   '*',   0,   ' ',   0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0
};

static const char scancode_to_ascii_shift[128] = {
    0,    27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',
   '(',  ')',  '_',  '+',  '\b', '\t', 'Q',  'W',  'E',  'R',
   'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}',  '\n',  0,
   'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
   '"',  '~',   0,   '|',  'Z',  'X',  'C',  'V',  'B',  'N',
   'M',  '<',  '>',  '?',   0,   '*',   0,   ' ',   0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0
};

#endif // KEYBOARD_H

/*
 * @file kernel.c
 * @version 0.0.7
 * LNL Kernel - Simple VGA/Framebuffer switch
 */

#include "keyboard.h"
#include "cursor.h"
#include "timer.h"
#include "vga.h"
#include "framebuffer.h"
#include "font.h"
#include "LNLISP/lnlisp.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

#define USE_FRAMEBUFFER 0  // 0 = VGA text mode, 1 = VESA framebuffer

#if USE_FRAMEBUFFER
// Framebuffer mode variables
static uint32_t text_x = 0;
static uint32_t text_y = 0;
static const uint32_t char_width = 8;
static const uint32_t char_height = 16;
static Color current_fg = COLOR_WHITE;
static Color current_bg = COLOR_BLACK;
#endif

void clear_screen(void) {
#if USE_FRAMEBUFFER
    framebuffer_clear(COLOR_BLACK);
    text_x = 0;
    text_y = 0;
#else
    uint8_t default_color = vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint16_t blank = vga_entry(' ', default_color);
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = blank;
    }
    cursor_set_position(0, 0);
#endif
}

void scroll_screen(void) {
#if USE_FRAMEBUFFER
    FramebufferInfo *fb = framebuffer_get_info();

    // Move all lines up by char_height pixels
    for (uint32_t y = char_height; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            Color pixel = framebuffer_getpixel(x, y);
            framebuffer_putpixel(x, y - char_height, pixel);
        }
    }

    // Clear last line
    uint32_t start_y = fb->height - char_height;
    for (uint32_t y = start_y; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            framebuffer_putpixel(x, y, COLOR_BLACK);
        }
    }
#else
    // Move all lines up
    for (uint32_t y = 1; y < VGA_HEIGHT; y++) {
        for (uint32_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[(y-1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }
    // Clear last line
    uint8_t default_color = vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint16_t blank = vga_entry(' ', default_color);
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT-1) * VGA_WIDTH + x] = blank;
    }
#endif
}

void putchar_at(char c, uint8_t color, uint32_t x, uint32_t y) {
    const uint32_t index = y * VGA_WIDTH + x;
    VGA_MEMORY[index] = vga_entry(c, color);
}

void putchar(char c) {
#if USE_FRAMEBUFFER
    FramebufferInfo *fb = framebuffer_get_info();
    uint32_t max_cols = fb->width / char_width;
    uint32_t max_rows = fb->height / char_height;

    if (c == '\n') {
        text_x = 0;
        text_y++;
        if (text_y >= max_rows) {
            text_y = max_rows - 1;
            scroll_screen();
        }
        return;
    }

    if (c == '\b') {
        if (text_x > 0) {
            text_x--;
            font_draw_char_builtin(text_x * char_width, text_y * char_height,
                                  ' ', current_fg, current_bg);
        }
        return;
    }

    font_draw_char_builtin(text_x * char_width, text_y * char_height,
                          c, current_fg, current_bg);

    text_x++;
    if (text_x >= max_cols) {
        text_x = 0;
        text_y++;
        if (text_y >= max_rows) {
            text_y = max_rows - 1;
            scroll_screen();
        }
    }
#else
    // Restore character at cursor position before writing new character
    cursor_restore_char();

    uint8_t default_color = vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
            scroll_screen();
        }
        cursor_reset_blink();
        cursor_update();
        return;
    }

    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            putchar_at(' ', default_color, cursor_x, cursor_y);
        }
        cursor_reset_blink();
        cursor_update();
        return;
    }

    putchar_at(c, default_color, cursor_x, cursor_y);
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
            scroll_screen();
        }
    }
    cursor_reset_blink();
    cursor_update();
#endif
}

void print(const char* str) {
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

#if USE_FRAMEBUFFER
void print_colored(const char* str, Color fg, Color bg) {
    Color old_fg = current_fg;
    Color old_bg = current_bg;

    current_fg = fg;
    current_bg = bg;

    print(str);

    current_fg = old_fg;
    current_bg = old_bg;
}
#else
// Dummy function for VGA mode
void print_colored(const char* str, Color fg, Color bg) {
    (void)fg;
    (void)bg;
    print(str);
}
#endif

void print_hex(uint32_t num) {
    const char hex[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        buf[i + 2] = hex[num & 0xF];
        num >>= 4;
    }
    buf[10] = '\0';
    print(buf);
}

void kernel_main(void) {
#if USE_FRAMEBUFFER
    // Initialize framebuffer with QEMU default address
    FramebufferInfo *fb = framebuffer_get_info();
    fb->buffer = (uint32_t*)0xFD000000;  // QEMU default
    fb->width = 1024;
    fb->height = 768;
    fb->pitch = 1024 * 4;
    fb->bpp = 32;

    font_init();
    clear_screen();

    print_colored("LNL Kernel v0.0.7 (Framebuffer)\n", COLOR_CYAN, COLOR_BLACK);
    print_colored("================================\n\n", COLOR_CYAN, COLOR_BLACK);

    print("FB: ");
    print_hex((uint32_t)fb->buffer);
    print(" @ ");
    print_hex(fb->width);
    print("x");
    print_hex(fb->height);
    print("\n\n");
#else
    // VGA text mode
    clear_screen();

    print("LNL Kernel v0.0.7 (VGA Text)\n");
    print("============================\n\n");
#endif

    print("Initializing interrupts...\n");

    // Initialize IDT and PIC
    idt_init(idt, &idtp);
    pic_init();

#if !USE_FRAMEBUFFER
    // Initialize cursor BEFORE timer so it's ready when timer starts
    cursor_init();
#endif

    timer_init();

#if USE_FRAMEBUFFER
    print_colored("Interrupts initialized.\n", COLOR_GREEN, COLOR_BLACK);
    print_colored("Timer initialized.\n", COLOR_GREEN, COLOR_BLACK);
    print_colored("Keyboard enabled.\n\n", COLOR_GREEN, COLOR_BLACK);
#else
    print("Interrupts initialized.\n");
    print("Timer initialized.\n");
    print("Keyboard enabled.\n\n");

    cursor_set_style(CURSOR_BLOCK);
    cursor_show();  // Explicitly show cursor
#endif

    lnlisp_init();
    lnlisp_repl();

    // Enable interrupts
    __asm__ volatile("sti");

    // Main loop
    while (1) {
        if (keyboard_has_input()) {
            char c = keyboard_getchar();
            if (c) {
                lnlisp_repl_input(c);
            }
        }
        __asm__ volatile("hlt");
    }
}

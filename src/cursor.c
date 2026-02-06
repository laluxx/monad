/*
 * @file cursor.c
 * @version 0.0.3
 * Software cursor with blinking
 */

#include "cursor.h"

// Port I/O
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Global cursor position
uint32_t cursor_x = 0;
uint32_t cursor_y = 0;

// Cursor state
static CursorStyle current_style = CURSOR_BLOCK;
static int cursor_visible = 1;
static int cursor_blink_visible = 1;

// Blink timing: 500ms visible, 500ms hidden (assuming ~18.2 Hz timer)
// At 18.2 Hz, 500ms ≈ 9 ticks
#define BLINK_TICKS 9

// Blink counter and limits
static uint32_t blink_count = 0;          // Number of complete blinks so far
static uint32_t blink_tick_counter = 0;   // Ticks since last blink toggle
static uint32_t blink_toggle_count = 0;   // Number of state toggles (2 toggles = 1 complete blink)
static int blink_cursor_mode = 1;
static int blink_cursor_blinks = 10;

// Saved character and position
static uint16_t saved_char = 0;
static uint32_t saved_x = 0;
static uint32_t saved_y = 0;
static int char_saved = 0;

// Restore character at saved position
static void restore_saved_char(void) {
    if (char_saved) {
        uint32_t pos = saved_y * VGA_WIDTH + saved_x;
        VGA_MEMORY[pos] = saved_char;
        char_saved = 0;
    }
}

void cursor_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    char_saved = 0;
    cursor_blink_visible = 1;
    blink_tick_counter = 0;
    blink_count = 0;

    // Disable hardware cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);  // Bit 5 = disable cursor

    cursor_set_style(CURSOR_BLOCK);
}

void cursor_set_position(uint32_t x, uint32_t y) {
    // Restore character at old position
    restore_saved_char();

    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;

    cursor_x = x;
    cursor_y = y;

    cursor_reset_blink();
    cursor_update();
}

void cursor_get_position(uint32_t *x, uint32_t *y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

void cursor_move(int dx, int dy) {
    // Restore character at old position
    restore_saved_char();

    int new_x = (int)cursor_x + dx;
    int new_y = (int)cursor_y + dy;

    if (new_x < 0) new_x = 0;
    if (new_x >= VGA_WIDTH) new_x = VGA_WIDTH - 1;
    if (new_y < 0) new_y = 0;
    if (new_y >= VGA_HEIGHT) new_y = VGA_HEIGHT - 1;

    cursor_x = (uint32_t)new_x;
    cursor_y = (uint32_t)new_y;

    cursor_reset_blink();
    cursor_update();
}

void cursor_set_style(CursorStyle style) {
    current_style = style;
    cursor_reset_blink();
    cursor_update();
}

void cursor_reset_blink(void) {
    blink_tick_counter = 0;
    blink_toggle_count = 0;       // Reset toggle count on keypress (Emacs behavior)
    cursor_blink_visible = 1;     // Show cursor immediately
    cursor_update();
}

void cursor_update(void) {
    uint32_t pos = cursor_y * VGA_WIDTH + cursor_x;

    if (!cursor_visible || current_style == CURSOR_HIDDEN) {
        // Restore character if hidden
        restore_saved_char();
        return;
    }

    // Check if cursor moved to a new position
    if (!char_saved || saved_x != cursor_x || saved_y != cursor_y) {
        // Restore old position first
        restore_saved_char();

        // Save new position
        saved_char = VGA_MEMORY[pos];
        saved_x = cursor_x;
        saved_y = cursor_y;
        char_saved = 1;
    }

    // Draw or hide cursor based on blink state
    if (cursor_blink_visible) {
        // Draw cursor (inverted)
        if (current_style == CURSOR_BLOCK) {
            VGA_MEMORY[pos] = vga_invert_colors(saved_char);
        } else if (current_style == CURSOR_UNDERLINE) {
            // For underline, we could modify the character to show underscore
            // For now, just invert like block
            VGA_MEMORY[pos] = vga_invert_colors(saved_char);
        }
    } else {
        // Blink off - show original character
        VGA_MEMORY[pos] = saved_char;
    }
}

void cursor_tick(void) {
    if (!cursor_visible || current_style == CURSOR_HIDDEN) return;

    // Check if blinking is disabled
    if (!blink_cursor_mode) {
        // Ensure cursor stays visible
        if (!cursor_blink_visible) {
            cursor_blink_visible = 1;
            cursor_update();
        }
        return;
    }

    // Check if we've exceeded max blinks (0 = infinite)
    // One complete blink = 2 toggles (visible->hidden->visible)
    if (blink_cursor_blinks > 0 && blink_toggle_count >= (blink_cursor_blinks * 2)) {
        // Stop blinking, keep cursor visible
        if (!cursor_blink_visible) {
            cursor_blink_visible = 1;
            cursor_update();
        }
        return;
    }

    blink_tick_counter++;
    if (blink_tick_counter >= BLINK_TICKS) {
        blink_tick_counter = 0;
        cursor_blink_visible = !cursor_blink_visible;
        blink_toggle_count++;  // Count each state change
        cursor_update();
    }
}

void cursor_show(void) {
    cursor_visible = 1;
    cursor_reset_blink();
    cursor_update();
}

void cursor_hide(void) {
    // Restore character before hiding
    restore_saved_char();
    cursor_visible = 0;
}

void cursor_restore_char(void) {
    // Public API to restore character at cursor position
    restore_saved_char();
}

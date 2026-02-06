/*
 * @file cursor.h
 * @version 0.0.3
 * Software cursor with blinking
 */

#ifndef CURSOR_H
#define CURSOR_H

#include "vga.h"

typedef enum {
    CURSOR_UNDERLINE,
    CURSOR_BLOCK,
    CURSOR_HIDDEN
} CursorStyle;

extern uint32_t cursor_x;
extern uint32_t cursor_y;

void cursor_init(void);
void cursor_set_position(uint32_t x, uint32_t y);
void cursor_get_position(uint32_t *x, uint32_t *y);
void cursor_move(int dx, int dy);
void cursor_set_style(CursorStyle style);
void cursor_show(void);
void cursor_hide(void);
void cursor_reset_blink(void);
void cursor_tick(void);
void cursor_update(void);
void cursor_restore_char(void);


#endif // CURSOR_H

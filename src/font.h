/*
 * @file font.h
 * @version 0.0.1
 * Font rendering system with TrueType support
 */

#ifndef FONT_H
#define FONT_H

#include "framebuffer.h"
#include <inttypes.h>

typedef struct {
    uint32_t codepoint;
    uint8_t *bitmap;     // Grayscale bitmap (1 byte per pixel)
    uint32_t width;
    uint32_t height;
    int32_t  bearing_x;  // Horizontal bearing
    int32_t  bearing_y;  // Vertical bearing
    uint32_t advance;    // Horizontal advance
} Glyph;

typedef struct {
    uint8_t *ttf_data;   // TTF file data in memory
    uint32_t ttf_size;   // Size of TTF data
    uint32_t font_size;  // Font size in pixels
    int32_t ascent;      // Font ascender
    int32_t descent;     // Font descender
    int32_t line_gap;    // Line gap

    // Glyph cache (simple fixed-size cache)
    Glyph glyph_cache[256];
    uint32_t cache_size;
} Font;

int font_init(void);
Font* font_load_ttf(const uint8_t *ttf_data, uint32_t size, uint32_t font_size);
void font_free(Font *font);
void font_draw_char(Font *font, uint32_t x, uint32_t y, char c, Color fg, Color bg);
void font_draw_string(Font *font, uint32_t x, uint32_t y, const char *text, Color fg, Color bg);
uint32_t font_string_width(Font *font, const char *text);
uint32_t font_get_height(Font *font);
void font_set_default(Font *font);
Font* font_get_default(void);

// Built-in 8x16 bitmap font (fallback)
extern const uint8_t builtin_font_8x16[256][16];
void font_draw_char_builtin(uint32_t x, uint32_t y, char c, Color fg, Color bg);
void font_draw_string_builtin(uint32_t x, uint32_t y, const char *text, Color fg, Color bg);

#endif // FONT_H

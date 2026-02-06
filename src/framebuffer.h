/*
 * @file framebuffer.h
 * @version 0.0.2
 * Framebuffer graphics mode support
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
    uint32_t *buffer; // Framebuffer address
    uint32_t width;   // Width in pixels
    uint32_t height;  // Height in pixels
    uint32_t pitch;   // Bytes per scanline
    uint32_t bpp;     // Bits per pixel (32)
} FramebufferInfo;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

// Predefined colors
#define COLOR_BLACK       ((Color){0, 0, 0, 255})
#define COLOR_WHITE       ((Color){255, 255, 255, 255})
#define COLOR_RED         ((Color){255, 0, 0, 255})
#define COLOR_GREEN       ((Color){0, 255, 0, 255})
#define COLOR_BLUE        ((Color){0, 0, 255, 255})
#define COLOR_YELLOW      ((Color){255, 255, 0, 255})
#define COLOR_CYAN        ((Color){0, 255, 255, 255})
#define COLOR_MAGENTA     ((Color){255, 0, 255, 255})
#define COLOR_ORANGE      ((Color){255, 165, 0, 255})
#define COLOR_PURPLE      ((Color){128, 0, 128, 255})
#define COLOR_GRAY        ((Color){128, 128, 128, 255})
#define COLOR_LIGHT_GRAY  ((Color){192, 192, 192, 255})
#define COLOR_DARK_GRAY   ((Color){64, 64, 64, 255})

// Syntax highlighting colors
#define COLOR_KEYWORD     ((Color){86, 156, 214, 255})   // Light blue
#define COLOR_STRING      ((Color){206, 145, 120, 255})  // Orange
#define COLOR_NUMBER      ((Color){181, 206, 168, 255})  // Light green
#define COLOR_COMMENT     ((Color){106, 153, 85, 255})   // Green
#define COLOR_FUNCTION    ((Color){220, 220, 170, 255})  // Light yellow
#define COLOR_PAREN       ((Color){218, 218, 170, 255})  // Yellow

int framebuffer_init(void);
FramebufferInfo* framebuffer_get_info(void);

void framebuffer_clear(Color color);
void framebuffer_putpixel(uint32_t x, uint32_t y, Color color);
Color framebuffer_getpixel(uint32_t x, uint32_t y);

void framebuffer_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Color color);
void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Color color);

uint32_t color_to_uint32(Color color);
Color uint32_to_color(uint32_t pixel);

#endif // FRAMEBUFFER_H

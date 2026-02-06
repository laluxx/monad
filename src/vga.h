/*
 * @file vga.h
 * @version 0.0.1
 */

#ifndef VGA_H
#define VGA_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// VGA text mode dimensions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// VGA memory address
#define VGA_MEMORY ((uint16_t*)0xB8000)

// VGA color codes
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} VGAColor;


uint8_t vga_color(VGAColor fg, VGAColor bg);
uint16_t vga_entry(char c, uint8_t color);
uint16_t vga_invert_colors(uint16_t vga_entry);


#endif // VGA_H

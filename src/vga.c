/*
 * @file vga.c
 * @version 0.0.1
 * VGA text mode definitions and utilities
 */

#include "vga.h"

// Create VGA color attribute byte
inline uint8_t vga_color(VGAColor fg, VGAColor bg) {
    return fg | (bg << 4);
}

// Create VGA entry (character + color)
inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

// Invert colors in a VGA entry
inline uint16_t vga_invert_colors(uint16_t vga_entry) {
    uint8_t ch = vga_entry & 0xFF;
    uint8_t fg = (vga_entry >> 8) & 0x0F;
    uint8_t bg = (vga_entry >> 12) & 0x0F;
    return (uint16_t)ch | ((uint16_t)bg << 8) | ((uint16_t)fg << 12);
}

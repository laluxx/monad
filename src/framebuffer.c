/*
 * @file framebuffer.c
 * @version 0.0.2
 * Framebuffer graphics implementation
 */

#include "framebuffer.h"

// Global framebuffer info
static FramebufferInfo fb_info = {0};

FramebufferInfo* framebuffer_get_info(void) {
    return &fb_info;
}

int framebuffer_init(void) {
    // This is just a stub - actual initialization happens in vesa_init_framebuffer
    return 0;
}

uint32_t color_to_uint32(Color color) {
    // RGBA to 0xAARRGGBB format
    return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
}

Color uint32_to_color(uint32_t pixel) {
    Color color;
    color.b = pixel & 0xFF;
    color.g = (pixel >> 8) & 0xFF;
    color.r = (pixel >> 16) & 0xFF;
    color.a = (pixel >> 24) & 0xFF;
    return color;
}

void framebuffer_putpixel(uint32_t x, uint32_t y, Color color) {
    if (!fb_info.buffer || x >= fb_info.width || y >= fb_info.height) {
        return;
    }

    uint32_t pixel = color_to_uint32(color);
    uint32_t offset = y * (fb_info.pitch / 4) + x;
    fb_info.buffer[offset] = pixel;
}

Color framebuffer_getpixel(uint32_t x, uint32_t y) {
    if (!fb_info.buffer || x >= fb_info.width || y >= fb_info.height) {
        return COLOR_BLACK;
    }

    uint32_t offset = y * (fb_info.pitch / 4) + x;
    uint32_t pixel = fb_info.buffer[offset];
    return uint32_to_color(pixel);
}

void framebuffer_clear(Color color) {
    if (!fb_info.buffer) {
        return;
    }

    uint32_t pixel = color_to_uint32(color);
    uint32_t total_pixels = (fb_info.pitch / 4) * fb_info.height;

    for (uint32_t i = 0; i < total_pixels; i++) {
        fb_info.buffer[i] = pixel;
    }
}

void framebuffer_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Color color) {
    // Top and bottom
    for (uint32_t i = 0; i < width; i++) {
        framebuffer_putpixel(x + i, y, color);
        framebuffer_putpixel(x + i, y + height - 1, color);
    }

    // Left and right
    for (uint32_t i = 0; i < height; i++) {
        framebuffer_putpixel(x, y + i, color);
        framebuffer_putpixel(x + width - 1, y + i, color);
    }
}

void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Color color) {
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            framebuffer_putpixel(x + i, y + j, color);
        }
    }
}

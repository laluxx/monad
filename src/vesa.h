/*
 * @file vesa.h
 * @version 0.0.1
 * VESA VBE structures and functions
 */

#ifndef VESA_H
#define VESA_H

#include "framebuffer.h"

// VESA function numbers
#define VESA_GET_INFO       0x4F00
#define VESA_GET_MODE_INFO  0x4F01
#define VESA_SET_MODE       0x4F02

// VESA mode attributes
#define VESA_MODE_SUPPORTED    (1 << 0)
#define VESA_MODE_GRAPHICS     (1 << 4)
#define VESA_MODE_LINEAR_FB    (1 << 7)

// Common VESA modes
#define VESA_MODE_640x480x32   0x112
#define VESA_MODE_800x600x32   0x115
#define VESA_MODE_1024x768x32  0x118
#define VESA_MODE_1280x1024x32 0x11B

// VESA info structures (must be in real mode accessible memory)
struct vbe_info {
    char     signature[4]; // "VESA"
    uint16_t version;
    uint32_t oem_string_ptr;
    uint32_t capabilities;
    uint32_t video_modes;
    uint16_t total_memory;
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;
    uint8_t  reserved[222];
    uint8_t  oem_data[256];
} __attribute__((packed));

struct vbe_mode_info {
    uint16_t attributes;
    uint8_t  window_a;
    uint8_t  window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t  w_char;
    uint8_t  y_char;
    uint8_t  planes;
    uint8_t  bpp;
    uint8_t  banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  image_pages;
    uint8_t  reserved0;
    uint8_t  red_mask;
    uint8_t  red_position;
    uint8_t  green_mask;
    uint8_t  green_position;
    uint8_t  blue_mask;
    uint8_t  blue_position;
    uint8_t  reserved_mask;
    uint8_t  reserved_position;
    uint8_t  direct_color_attributes;
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t  reserved1[206];
} __attribute__((packed));

int vesa_init_framebuffer(struct vbe_mode_info *mode_info);

#endif // VESA_H

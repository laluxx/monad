/*
 * @file vesa.c
 * @version 0.0.1
 * VESA VBE mode setup
 */

#include "vesa.h"

// Initialize framebuffer from VESA mode info
// This is called after entering protected mode
int vesa_init_framebuffer(struct vbe_mode_info *mode_info) {
    FramebufferInfo *fb = framebuffer_get_info();

    if (!mode_info) {
        return -1;
    }

    fb->buffer = (uint32_t*)mode_info->framebuffer;
    fb->width = mode_info->width;
    fb->height = mode_info->height;
    fb->pitch = mode_info->pitch;
    fb->bpp = mode_info->bpp;

    return 0;
}

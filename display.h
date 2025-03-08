#ifndef DISPLAY_H
#define DISPLAY_H

#include <stddef.h>
#include <stdint.h>

struct display {
    size_t xres, yres;
    void *framebuffer;
    uint32_t *buffer;
};

struct color {
    uint8_t a, r, g, b;
};

struct display display_init();
void render_frame(struct display disp);
void set_pixel(struct display disp, size_t y, size_t x, struct color col);
void display_free(struct display disp);

#endif

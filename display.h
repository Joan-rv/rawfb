#ifndef DISPLAY_H
#define DISPLAY_H

#include <linux/fb.h>
#include <stddef.h>
#include <stdint.h>

struct display {
    size_t xres, yres;
    void *framebuffer;
    uint32_t *buffer;
    int fd;
};

struct color {
    uint8_t a, r, g, b;
};

struct display display_init();
void display_render_frame(struct display disp);
void display_set_pixel(struct display disp, size_t y, size_t x,
                       struct color col);
struct color display_get_pixel(struct display disp, size_t y, size_t x);
void display_clear(struct display disp, struct color clear_col);
void display_free(struct display disp);

#endif

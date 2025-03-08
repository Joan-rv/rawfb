#ifndef DISPLAY_H
#define DISPLAY_H

#include <stddef.h>
#include <stdint.h>

struct display {
    size_t xres, yres;
    void *framebuffer;
    uint32_t *buffer;
};

struct display display_init();
void render_frame(struct display disp);
void display_free(struct display disp);

#endif

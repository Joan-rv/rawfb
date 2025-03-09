#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "display.h"

uint32_t color_to_pixel(struct color col) {
    return col.a << 24 | col.r << 16 | col.g << 8 | col.b;
}

struct display display_init() {
    struct display disp = {
        .xres = 0, .yres = 0, .framebuffer = NULL, .buffer = NULL};
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        perror("Failed to open fb0");
        return disp;
    }

    struct fb_var_screeninfo screeninfo;
    int retval = ioctl(fb_fd, FBIOGET_VSCREENINFO, &screeninfo);
    if (retval < 0) {
        perror("Failed to call screeninfo ioctl");
        close(fb_fd);
        return disp;
    }

    void *framebuffer = mmap(NULL, 4 * screeninfo.xres * screeninfo.yres,
                             PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (framebuffer == MAP_FAILED) {
        perror("Failed to memory map framebuffer");
        close(fb_fd);
        return disp;
    }
    close(fb_fd);

    uint32_t *buffer = malloc(4 * screeninfo.xres * screeninfo.yres);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return disp;
    }

    disp.xres = screeninfo.xres;
    disp.yres = screeninfo.yres;
    disp.framebuffer = framebuffer;
    disp.buffer = buffer;
    return disp;
}

void display_render_frame(struct display disp) {
    memcpy(disp.framebuffer, disp.buffer, 4 * disp.xres * disp.yres);
}

void display_set_pixel(struct display disp, size_t y, size_t x,
                       struct color col) {
    disp.buffer[y * disp.xres + x] = color_to_pixel(col);
}

struct color display_get_pixel(struct display disp, size_t y, size_t x) {
    uint32_t pixel = disp.buffer[y * disp.xres + x];
    struct color col = {pixel >> 24, pixel >> 16, pixel >> 8, pixel};
    return col;
}

void display_clear(struct display disp, struct color clear_col) {
    uint32_t pixel = color_to_pixel(clear_col);
    for (size_t i = 0; i < disp.xres * disp.yres; i++) {
        disp.buffer[i] = pixel;
    }
}

void display_free(struct display disp) {
    free(disp.buffer);
    munmap(disp.framebuffer, 4 * disp.xres * disp.yres);
}

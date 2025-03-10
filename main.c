#include <fcntl.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "display.h"
#include "keyboard.h"

void enable_graphics_mode() {
    int fd = open("/dev/console", O_WRONLY);
    if (fd == -1) {
        perror("Failed to open console");
        return;
    }
    int retval = ioctl(fd, KDSETMODE, KD_GRAPHICS);
    if (retval < 0) {
        perror("Failed ioctl to set graphics mode");
    }
    close(fd);
}

void disable_graphics_mode() {
    int fd = open("/dev/console", O_WRONLY);
    if (fd == -1) {
        perror("Failed to open console");
        return;
    }
    int retval = ioctl(fd, KDSETMODE, KD_TEXT);
    if (retval < 0) {
        perror("Failed ioctl to set text mode");
    }
    close(fd);
}

void draw_circle(struct display disp) {
    struct color white = {0xff, 0xff, 0xff, 0xff};
    size_t r = 100;
    for (double theta = 0; theta < 2 * M_PI; theta += 0.01f) {
        size_t x = r * cos(theta);
        size_t y = r * sin(theta);
        display_set_pixel(disp, disp.yres / 2 + x, disp.xres / 2 + y, white);
    }
}

int main() {
    struct display disp = display_init();
    if (disp.framebuffer == NULL) {
        perror("Failed to initialize display");
        return -1;
    }

    // enable_graphics_mode();

    struct fd_vec keyboard_fds = find_keyboards();

    bool running = true;
    struct color black = {0xff, 0x00, 0x00, 0x00};
    while (running) {
        display_clear(disp, black);
        draw_circle(disp);
        display_render_frame(disp);

        struct input_event kb_event;
        while (poll_keyboards(keyboard_fds, &kb_event)) {
            if (kb_event.code == KEY_ESC) {
                running = false;
            }
        }

        usleep(10000);
    }

    free_keyboards(keyboard_fds);
    display_free(disp);
    disable_graphics_mode();

    return 0;
}

#include <bits/time.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
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

void draw_circle(struct display disp, double dt) {
    struct color white = {0xff, 0xff, 0xff, 0xff};
    static double x_0 = 100.0f;
    static double y_0 = 100.0f;
    x_0 += 10 * dt;
    y_0 += 10 * dt;
    size_t r = 100;
    for (double theta = 0; theta < 2 * M_PI; theta += 0.01f) {
        size_t x = r * cos(theta);
        size_t y = r * sin(theta);
        display_set_pixel(disp, (size_t)y_0 + y, (size_t)x_0 + x, white);
    }
}

double time_as_double(clockid_t clockid) {
    struct timespec tp;
    if (clock_gettime(clockid, &tp) < 0) {
        perror("Failed to get clock time");
    }
    return tp.tv_sec + (double)tp.tv_nsec / 1000000000;
}

int main() {
    struct display disp = display_init();
    if (disp.framebuffer == NULL) {
        perror("Failed to initialize display");
        return -1;
    }

    // enable_graphics_mode();

    struct fd_vec keyboard_fds = find_keyboards();

    double time_prev = time_as_double(CLOCK_MONOTONIC);
    bool running = true;
    struct color black = {0xff, 0x00, 0x00, 0x00};
    while (running) {
        double time_curr = time_as_double(CLOCK_MONOTONIC);
        double dt = time_curr - time_prev;
        time_prev = time_curr;
        display_clear(disp, black);
        draw_circle(disp, dt);
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

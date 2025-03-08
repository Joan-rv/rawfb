#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main() {
    struct display disp = display_init();
    if (disp.framebuffer == NULL) {
        perror("Failed to initialize display");
        return -1;
    }

    // enable_graphics_mode();

    for (size_t i = 0; i < disp.yres; i++) {
        for (size_t j = 0; j < disp.xres; j++) {
            uint8_t r = 255 * i / disp.yres;
            // printf("i = %lu, r = %hhu", i, r);
            uint8_t g = r;
            uint8_t b = r;
            disp.buffer[i * disp.xres + j] = 0xff << 24 | r << 16 | g << 8 | b;
        }
    }

    struct fd_vec keyboard_fds = find_keyboards();

    bool running = true;
    while (running) {
        render_frame(disp);

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

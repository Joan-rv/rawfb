#include <errno.h>
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
    int fb_fd = open("/dev/fb0", O_WRONLY);
    if (fb_fd < 0) {
        perror("Failed to open fb0");
        return -1;
    }

    struct fb_var_screeninfo screeninfo;
    int retval = ioctl(fb_fd, FBIOGET_VSCREENINFO, &screeninfo);
    if (retval < 0) {
        perror("Failed to call screeninfo ioctl");
        close(fb_fd);
        return -1;
    }
    size_t fb_size = 4 * screeninfo.xres * screeninfo.yres;

    enable_graphics_mode();

    uint32_t *buf = malloc(fb_size);
    if (buf == NULL) {
        perror("Failed to allocate memory");
        close(fb_fd);
        return -1;
    }
    for (size_t i = 0; i < fb_size / 4; i++) {
        uint8_t r = i % 255;
        uint8_t g = (i / 255) % 255;
        uint8_t b = (i / 255 / 255) % 255;
        buf[i] = 0xff << 24 | r << 16 | g << 8 | b;
    }

    struct fd_vec keyboard_fds = find_keyboards();

    bool running = true;
    while (running) {
        if (write(fb_fd, buf, fb_size) < 0) {
            perror("Failed to write to framebuffer");
            free(buf);
            close(fb_fd);
            return -1;
        }
        if (lseek(fb_fd, 0, SEEK_SET) < 0) {
            perror("Failed to seek to framebuffer beggining");
            free(buf);
            close(fb_fd);
        }

        struct input_event kb_event;
        while (poll_keyboards(keyboard_fds, &kb_event)) {
            if (kb_event.code == KEY_ESC) {
                running = false;
            }
        }

        usleep(10000);
    }

    free_keyboards(keyboard_fds);
    disable_graphics_mode();

    free(buf);
    close(fb_fd);
    return 0;
}

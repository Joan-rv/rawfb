#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

void enable_graphics_mode() {
    int fd = open("/dev/console", O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "Failed to open console\n%s\n", strerror(errno));
        return;
    }
    int retval = ioctl(fd, KDSETMODE, KD_GRAPHICS);
    if (retval < 0) {
        fprintf(stderr, "Failed ioctl to set graphics mode\n%s\n",
                strerror(errno));
    }
    close(fd);
}

int main() {
    int fb_fd = open("/dev/fb0", O_WRONLY);
    if (fb_fd < 0) {
        fprintf(stderr, "Failed to open fb0\n%s\n", strerror(errno));
        return -1;
    }

    struct fb_var_screeninfo screeninfo;
    int retval = ioctl(fb_fd, FBIOGET_VSCREENINFO, &screeninfo);
    if (retval < 0) {
        fprintf(stderr, "Failed to call screeninfo ioctl\n%s\n",
                strerror(errno));
        close(fb_fd);
        return -1;
    }
    size_t fb_size = 4 * screeninfo.xres * screeninfo.yres;

    enable_graphics_mode();

    uint32_t *buf = malloc(fb_size);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory\n%s\n", strerror(errno));
        close(fb_fd);
        return -1;
    }
    for (size_t i = 0; i < fb_size / 4; i++) {
        uint8_t r = i % 255;
        uint8_t g = (i / 255) % 255;
        uint8_t b = (i / 255 / 255) % 255;
        buf[i] = 0xff << 24 | r << 16 | g << 8 | b;
    }

    while (1) {
        if (write(fb_fd, buf, fb_size) < 0) {
            fprintf(stderr, "Failed to write to framebuffer\n%s\n",
                    strerror(errno));
            free(buf);
            close(fb_fd);
            return -1;
        }
        if (lseek(fb_fd, 0, SEEK_SET) < 0) {
            fprintf(stderr, "Failed to seek to framebuffer beggining\n%s\n",
                    strerror(errno));
            free(buf);
            close(fb_fd);
        }
        usleep(10000);
    }

    free(buf);
    close(fb_fd);
    return 0;
}

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
    int fb_fd = open("/dev/fb0", O_WRONLY);
    if (fb_fd < 0) {
        fprintf(stderr, "Failed to open fb0\n%s\n", strerror(errno));
        return -1;
    }

    struct fb_var_screeninfo screeninfo;
    int retval = ioctl(fb_fd, FBIOGET_VSCREENINFO, &screeninfo);
    if (retval < 0) {
        fprintf(stderr, "Failed to call ioctl\n%s\n", strerror(errno));
        close(fb_fd);
        return -1;
    }
    size_t fb_size = 4 * screeninfo.xres * screeninfo.yres;
    struct fb_cursor cursor;
    memset(&cursor, 0, sizeof(struct fb_cursor));
    cursor.set = FB_CUR_SETALL;
    cursor.enable = false;
    ioctl(fb_fd, FBIO_CURSOR, &cursor);
    if (retval < 0) {
        fprintf(stderr, "Failed to call ioctl\n%s\n", strerror(errno));
        close(fb_fd);
        return -1;
    }

    char *buf = malloc(fb_size);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory\n%s\n", strerror(errno));
        close(fb_fd);
        return -1;
    }
    memset(buf, 0xff, fb_size);

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

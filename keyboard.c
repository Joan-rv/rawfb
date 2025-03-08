#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "keyboard.h"

int open_kb(DIR *kb_dir, char *kb_path) {
    int kb_fd = openat(dirfd(kb_dir), kb_path, O_RDONLY | O_NONBLOCK);
    if (kb_fd < 0) {
        perror("Failed to open keyboard");
        return -1;
    }

    unsigned long ev_bits[(EV_MAX + 7) / 8] = {0};
    if (ioctl(kb_fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) {
        perror("Failed to get keyboard info");
        close(kb_fd);
        return -2;
    }

    if (ev_bits[EV_KEY / 8] & (1 << (EV_KEY % 8))) {
        return kb_fd;
    }

    close(kb_fd);
    return -3;
}

bool fd_vec_push(struct fd_vec *vec, size_t *cap, int fd) {
    if (vec->size >= *cap) {
        *cap *= 2;
        vec->fds = realloc(vec->fds, *cap * sizeof(int));
        if (vec->fds == NULL) {
            perror("Memory allocation error");
            return false;
        }
    }

    vec->fds[vec->size] = fd;
    vec->size++;
    return true;
}

struct fd_vec find_keyboards() {
    size_t capacity = 5;
    struct fd_vec kb_fds = {.fds = malloc(5 * sizeof(int)), .size = 0};
    if (kb_fds.fds == NULL) {
        perror("Memory allocation error");
        return kb_fds;
    }

    DIR *kb_dir = opendir("/dev/input");
    if (kb_dir == NULL) {
        perror("Failed to open input directory");
        return kb_fds;
    }

    errno = 0;
    struct dirent *dir_entry;
    while ((dir_entry = readdir(kb_dir))) {
        if (strncmp(dir_entry->d_name, "event", 5) != 0)
            continue;

        int kb_fd = open_kb(kb_dir, dir_entry->d_name);
        if (kb_fd >= 0) {
            if (!fd_vec_push(&kb_fds, &capacity, kb_fd)) {
                closedir(kb_dir);
                return kb_fds;
            }
        }
    }

    if (errno != 0) {
        perror("Failed to enumerate input directory");
    }

    closedir(kb_dir);
    return kb_fds;
}

bool poll_keyboards(struct fd_vec kb_fds, struct input_event *out) {
    for (size_t i = 0; i < kb_fds.size; i++) {
        if (read(kb_fds.fds[i], out, sizeof(*out)) == sizeof(*out)) {
            return true;
        }
    }
    return false;
}

void free_keyboards(struct fd_vec kb_fds) {
    if (kb_fds.fds == NULL)
        return;
    for (size_t i = 0; i < kb_fds.size; i++) {
        close(kb_fds.fds[i]);
    }
    free(kb_fds.fds);
}

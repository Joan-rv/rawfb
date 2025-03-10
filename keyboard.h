#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stddef.h>

struct fd_vec {
    int *fds;
    size_t size;
};

struct fd_vec find_keyboards();
bool poll_keyboards(struct fd_vec kb_fds, struct input_event *out);
void free_keyboards(struct fd_vec kb_fds);

#endif

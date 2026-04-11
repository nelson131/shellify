#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

extern size_t window_cols;
extern size_t window_rows;

static char* actual_buffer = NULL;
static char* old_buffer = NULL;

static inline size_t to_index(size_t x, size_t y) {
    return y * window_cols + x;
}

void buffer_init(size_t w_cols, size_t w_rows);
void buffer_clear();
void buffer_destroy();

void buffer_render();
void buffer_render_full();
void buffer_set_char(size_t x, size_t y, char ch);

size_t buffer_get_cols();
size_t buffer_get_rows();

#endif

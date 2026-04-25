#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

#include "error_handler.h"

extern size_t window_cols;
extern size_t window_rows;

extern char* actual_buffer;
extern char* old_buffer;

static inline size_t to_index(size_t x, size_t y) {
    return y * window_cols + x;
}

int  buffer_init(size_t w_cols, size_t w_rows);
void buffer_clear();
void buffer_destroy();

void buffer_render();
void buffer_render_full();

void buffer_set_char(size_t x, size_t y, char ch);
void buffer_append_line(size_t x, size_t y, const char* line);
void buffer_append_vertical_line(size_t x, size_t y, const char* line);

void buffer_clear_line(size_t fx, size_t sx, size_t y);
void buffer_clear_vertical_line(size_t fy, size_t sy, size_t x);
void buffer_clear_square(size_t fx, size_t fy, size_t sx, size_t sy);

size_t buffer_get_cols();
size_t buffer_get_rows();

#endif

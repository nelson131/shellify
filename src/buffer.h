#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

#include "error_handler.h"
#include "vec.h"

typedef struct Buffer {
    char* actual;
    char* old;

    size_t window_cols;
    size_t window_rows;
} Buffer;

static size_t to_index(Buffer* buffer, Vec v);

int  buffer_init(Buffer** buffer, size_t* window_cols, size_t* window_rows);
void buffer_clear(Buffer* buffer);
void buffer_destroy(Buffer* buffer);

void buffer_render(Buffer* buffer);
void buffer_render_full(Buffer* buffer);

void buffer_set_char(Buffer* buffer, Vec v, char line);
void buffer_append_line(Buffer* buffer, Vec v, const char* line);
void buffer_append_vertical_line(Buffer* buffer, Vec v, const char* line);
void buffer_set_range_char(Buffer* buffer, Vec range, Vec v, char ch);
void buffer_set_ver_range_char(Buffer* buffer, Vec range, Vec v, char ch);
void buffer_append_range(Buffer* buffer, Vec range, Vec v, const char* line);
void buffer_append_ver_range(Buffer* buffer, Vec range, Vec v,
                             const char* line);

void buffer_clear_line(Buffer* buffer, Vec range, size_t y);
void buffer_clear_vertical_line(Buffer* buffer, Vec range, size_t x);
void buffer_clear_square(Buffer* buffer, Vec v1, Vec v2);

#endif

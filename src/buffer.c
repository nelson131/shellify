#include "buffer.h"

#include <stdio.h>

size_t to_index(Buffer* buffer, Vec v) {
    return v.y * buffer->window_cols + v.x;
}

int buffer_init(Buffer** buffer, size_t* window_cols, size_t* window_rows) {
    Buffer* temp = malloc(sizeof(Buffer));
    if (!temp) {
        raise_error(ERR_MALLOC_NULL, "buffer:init:buffer");
        return 0;
    }
    size_t size = *window_cols * *window_rows;
    temp->actual = malloc(size * sizeof(char));
    temp->old = malloc(size * sizeof(char));
    if (!temp->actual || !temp->old) {
        raise_error(ERR_MALLOC_NULL, "buffer:init:buffer_arrays");
        return 0;
    }

    memset(temp->actual, ' ', size);
    memset(temp->old, ' ', size);

    temp->window_cols = *window_cols;
    temp->window_rows = *window_rows;

    *buffer = temp;
    return 1;
}

void buffer_clear(Buffer* buffer) {
    memset(buffer->actual, ' ', buffer->window_cols * buffer->window_rows);
}

void buffer_destroy(Buffer* buffer) {
    if (buffer->actual) {
        free(buffer->actual);
        buffer->actual = NULL;
    }

    if (buffer->old) {
        free(buffer->old);
        buffer->old = NULL;
    }
}

void buffer_render(Buffer* buffer) {
    for (size_t y = 0; y < buffer->window_rows; y++) {
        for (size_t x = 0; x < buffer->window_cols; x++) {
            size_t i = to_index(buffer, (Vec){x, y});

            if (buffer->actual[i] != buffer->old[i]) {
                printf("\033[%zu;%zuH", y + 1, x + 1);
                putchar(buffer->actual[i]);
                buffer->old[i] = buffer->actual[i];
            }
        }
    }

    fflush(stdout);
}

void buffer_render_full(Buffer* buffer) {
    printf("\033[H");

    for (size_t y = 0; y < buffer->window_rows; y++) {
        for (size_t x = 0; x < buffer->window_cols; x++) {
            size_t i = to_index(buffer, (Vec){x, y});
            putchar(buffer->actual[i]);
            buffer->old[i] = buffer->actual[i];
        }
    }

    fflush(stdout);
}

void buffer_set_char(Buffer* buffer, Vec v, char ch) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;

    buffer->actual[to_index(buffer, v)] = ch;
}

void buffer_append_line(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    size_t len = strlen(line);
    if (v.x + len > buffer->window_cols) return;

    for (size_t i = 0; i < len; i++) {
        buffer->actual[to_index(buffer, (Vec){i + v.x, v.y})] = line[i];
    }
}

void buffer_append_vertical_line(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    size_t len = strlen(line);
    if (v.y + len > buffer->window_rows) return;

    for (size_t i = 0; i < len; i++) {
        buffer->actual[to_index(buffer, (Vec){v.x, v.y + i})] = line[i];
    }
}

void buffer_set_range_char(Buffer* buffer, Vec range, Vec v, char ch) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.x > buffer->window_cols) return;

    for (size_t i = range.x; i < range.y - range.x; i++) {
        buffer_set_char(buffer, (Vec){v.x + i, v.y}, ch);
    }
}

void buffer_set_ver_range_char(Buffer* buffer, Vec range, Vec v, char ch) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.y > buffer->window_cols) return;

    for (size_t i = range.x; i < range.y - range.x; i++) {
        buffer_set_char(buffer, (Vec){v.x, v.y + i}, ch);
    }
}

void buffer_append_range(Buffer* buffer, Vec range, Vec v, const char* line) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.x > buffer->window_cols) return;
    size_t len = strlen(line);
    if (len >= buffer->window_cols) return;

    size_t idx = 0;
    for (size_t i = range.x; i < range.y - range.x; i++, idx++) {
        buffer_set_char(buffer, (Vec){v.x + i, v.y}, line[idx]);
        if (idx >= len) idx = 0;
    }
}

void buffer_append_ver_range(Buffer* buffer, Vec range, Vec v,
                             const char* line) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.y > buffer->window_rows) return;
    size_t len = strlen(line);
    if (len >= buffer->window_rows) return;

    size_t idx = 0;
    for (size_t i = range.x; i < range.y; i++, idx++) {
        buffer_set_char(buffer, (Vec){v.x, v.y + i}, line[idx]);
        if (idx >= len - 1) idx = 0;
    }
}

void buffer_clear_line(Buffer* buffer, Vec range, size_t y) {
    if (range.y >= buffer->window_cols || y >= buffer->window_rows) return;

    for (size_t i = range.x; i <= range.y; i++) {
        size_t index = to_index(buffer, (Vec){i, y});
        if (buffer->actual[index] != ' ') {
            buffer->actual[index] = ' ';
        }
    }
}

void buffer_clear_vertical_line(Buffer* buffer, Vec range, size_t x) {
    if (range.y >= buffer->window_rows || x >= buffer->window_cols) return;

    for (size_t i = range.x; i <= range.y; i++) {
        size_t index = to_index(buffer, (Vec){x, i});
        if (buffer->actual[index] != ' ') {
            buffer->actual[index] = ' ';
        }
    }
}

void buffer_clear_square(Buffer* buffer, Vec v1, Vec v2) {
    if (v2.x >= buffer->window_cols || v2.y >= buffer->window_rows) return;

    for (size_t y = v1.y; y <= v2.y; y++) {
        buffer_clear_line(buffer, (Vec){v1.x, v2.y}, y);
    }
}

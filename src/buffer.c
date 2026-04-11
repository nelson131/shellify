#include "buffer.h"

#include <stdio.h>

size_t window_cols = 0;
size_t window_rows = 0;

void buffer_init(size_t w_cols, size_t w_rows) {
    window_cols = w_cols;
    window_rows = w_rows;

    size_t size = w_cols * w_rows;
    actual_buffer = malloc(size * sizeof(char));
    old_buffer = malloc(size * sizeof(char));
    if (!actual_buffer || !old_buffer) {
        exit(1);
    }

    memset(actual_buffer, ' ', size);
    memset(old_buffer, ' ', size);
}

void buffer_clear() { memset(actual_buffer, ' ', window_cols * window_rows); }

void buffer_destroy() {
    if (actual_buffer) {
        free(actual_buffer);
        actual_buffer = NULL;
    }

    if (old_buffer) {
        free(old_buffer);
        old_buffer = NULL;
    }
}

void buffer_render() {
    for (size_t y = 0; y < window_rows; y++) {
        for (size_t x = 0; x < window_cols; x++) {
            size_t i = to_index(x, y);

            if (actual_buffer[i] != old_buffer[i]) {
                printf("\033[%zu;%zuH", y + 1, x + 1);
                putchar(actual_buffer[i]);

                old_buffer[i] = actual_buffer[i];
            }
        }
    }

    fflush(stdout);
}

void buffer_render_full() {
    printf("\033[H");

    for (size_t y = 0; y < window_rows; y++) {
        for (size_t x = 0; x < window_cols; x++) {
            size_t i = to_index(x, y);
            putchar(actual_buffer[i]);
            old_buffer[i] = actual_buffer[i];
        }
    }

    fflush(stdout);
}

void buffer_set_char(size_t x, size_t y, char ch) {
    if (x >= window_cols || y >= window_rows) return;

    actual_buffer[to_index(x, y)] = ch;
}

size_t buffer_get_cols() { return window_cols; }

size_t buffer_get_rows() { return window_rows; }

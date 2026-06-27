#include "buffer.h"

size_t to_index(Buffer* buffer, Vec v) {
    return v.y * buffer->window_cols + v.x;
}

void cell_reset(Cell* cell) {
    if (!cell) return;

    cell->c = ' ';
    cell->tg = COLOR_DEFAULT;
    cell->bg = COLOR_DEFAULT;
    cell->fl = STYLE_NONE;
}

int cell_eq(Cell* c1, Cell* c2) {
    return (c1->c == c2->c && c1->tg == c2->tg && c1->bg == c2->bg &&
            c1->fl == c2->fl);
}

void cell_cmpl(Cell* c, i32* c_tg, i32* c_bg, u8* c_fl) {
    if (!c || !c_tg || !c_bg || !c_fl) return;

    if (c->tg != *c_tg || c->bg != *c_bg || c->fl != *c_fl) {
        printf("\033[0m");

        if (c->fl & STYLE_BOLD) {
            printf("\033[1m");
        }

        if (c->fl & STYLE_UNDERLINE) {
            printf("\033[4m");
        }

        if (c->tg != COLOR_DEFAULT) {
            printf("\033[%dm", 30 + c->tg);
        }

        if (c->bg != COLOR_DEFAULT) {
            printf("\033[%dm", 40 + c->bg);
        }

        *c_tg = c->tg;
        *c_bg = c->bg;
        *c_fl = c->fl;
    }
}

int buffer_init(Buffer** buffer, size_t* window_cols, size_t* window_rows) {
    if (!window_cols || !window_rows) return 0;

    Buffer* temp = malloc(sizeof(Buffer));
    if (!temp) {
        errlog(ERR_MALLOC_NULL, "buffer:init:temp");
        return 0;
    }

    size_t size = *window_cols * *window_rows;
    temp->size = size;

    temp->actual = malloc(size * sizeof(Cell));
    if (!temp->actual) {
        errlog(ERR_MALLOC_NULL, "buffer:init:actual");
        free(temp);
        return 0;
    }
    temp->old = malloc(size * sizeof(Cell));
    if (!temp->old) {
        errlog(ERR_MALLOC_NULL, "buffer:init:old");
        free(temp->actual);
        free(temp);
        return 0;
    }

    temp->window_cols = *window_cols;
    temp->window_rows = *window_rows;

    buffer_clear(temp);
    for (size_t i = 0; i < size; i++) {
        cell_reset(&temp->old[i]);
    }

    *buffer = temp;
    return 1;
}

void buffer_clear(Buffer* buffer) {
    if (!buffer || !buffer->actual) return;

    for (size_t i = 0; i < buffer->size; i++) {
        cell_reset(&buffer->actual[i]);
    }
}

void buffer_destroy(Buffer* buffer) {
    if (!buffer) return;

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
    i32 c_tg = 0;
    i32 c_bg = 0;
    u8  c_fl = 0;

    for (size_t y = 0; y < buffer->window_rows; y++) {
        for (size_t x = 0; x < buffer->window_cols; x++) {
            size_t i = to_index(buffer, (Vec){x, y});

            if (!cell_eq(&buffer->actual[i], &buffer->old[i])) {
                printf("\033[%zu;%zuH", y + 1, x + 1);
                cell_cmpl(&buffer->actual[i], &c_tg, &c_bg, &c_fl);

                putchar(buffer->actual[i].c);
                buffer->old[i] = buffer->actual[i];
            }
        }
    }

    printf("\033[0m");
    fflush(stdout);
}

void buffer_set_char(Buffer* buffer, Vec v, char ch) {
    buffer_set_cell(buffer, v, ch, COLOR_DEFAULT, COLOR_DEFAULT, STYLE_NONE);
}

void buffer_set_cell(Buffer* buffer, Vec v, char ch, i32 tg, i32 bg, u8 fl) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;

    size_t i = to_index(buffer, v);
    buffer->actual[i].c = ch;
    buffer->actual[i].tg = tg;
    buffer->actual[i].bg = bg;
    buffer->actual[i].fl = fl;
}

void buffer_append_line(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    size_t len = strlen(line);
    if (v.x + len > buffer->window_cols) {
        len = buffer->window_cols - v.x;
    }

    for (size_t i = 0; i < len; i++) {
        buffer_set_char(buffer, (Vec){i + v.x, v.y}, line[i]);
    }
}

void buffer_append_line_styled(Buffer* buffer, Vec v, const char* line, i32 tg,
                               i32 bg, u8 fl) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    size_t len = strlen(line);
    if (v.x + len > buffer->window_cols) {
        len = buffer->window_cols - v.x;
    }

    for (size_t i = 0; i < len; i++) {
        buffer_set_cell(buffer, (Vec){i + v.x, v.y}, line[i], tg, bg, fl);
    }
}

size_t buffer_append_line_offset(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return 0;

    size_t len = strlen(line);
    size_t offset = 0;
    for (size_t i = 0; i < len; i++) {
        if (v.x >= buffer->window_cols) {
            v.y++;
            v.x = 0;
            offset++;
        }
        buffer_set_char(buffer, (Vec){v.x++, v.y}, line[i]);
    }

    return offset;
}

void buffer_append_vertical_line(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    size_t len = strlen(line);
    if (v.y + len > buffer->window_rows) return;

    for (size_t i = 0; i < len; i++) {
        buffer_set_char(buffer, (Vec){v.x, v.y + i}, line[i]);
    }
}

void buffer_append_line_vertical_styled(Buffer* buffer, Vec v, const char* line,
                                        i32 tg, i32 bg, u8 fl) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    size_t len = strlen(line);
    if (v.y + len > buffer->window_rows) {
        len = buffer->window_rows - v.y;
    }

    for (size_t i = 0; i < len; i++) {
        buffer_set_cell(buffer, (Vec){v.x, v.y + i}, line[i], tg, bg, fl);
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
        if (buffer->actual[index].c != ' ') {
            cell_reset(&buffer->actual[index]);
        }
    }
}

void buffer_clear_vertical_line(Buffer* buffer, Vec range, size_t x) {
    if (range.y >= buffer->window_rows || x >= buffer->window_cols) return;

    for (size_t i = range.x; i <= range.y; i++) {
        size_t index = to_index(buffer, (Vec){x, i});
        if (buffer->actual[index].c != ' ') {
            cell_reset(&buffer->actual[index]);
        }
    }
}

void buffer_clear_square(Buffer* buffer, Vec v1, Vec v2) {
    if (v2.x >= buffer->window_cols || v2.y >= buffer->window_rows) return;

    for (size_t y = v1.y; y <= v2.y; y++) {
        buffer_clear_line(buffer, (Vec){v1.x, v2.y}, y);
    }
}

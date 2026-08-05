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

u32 utf8_decode(const char** c) {
    const unsigned char* s = (const unsigned char*)*c;

    uint32_t ch = *s;
    if (ch < 0x80) {
        *c += 1;
        return ch;
    } else if ((ch & 0xE0) == 0xC0) {
        ch = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        *c += 2;
    } else if ((ch & 0xF0) == 0xE0) {
        ch = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        *c += 3;
    } else if ((ch & 0xF8) == 0xF0) {
        ch = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) |
             ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        *c += 4;
    } else {
        *c += 1;
    }

    return ch;
}

static int utf8_encode(u32 c, char* out) {
    if (c <= 0x7F) {
        out[0] = (char)c;
        return 1;
    } else if (c <= 0x7FF) {
        out[0] = (char)(0xC0 | ((c >> 6) & 0x1F));
        out[1] = (char)(0x80 | (c & 0x3F));
        return 2;
    } else if (c <= 0xFFFF) {
        out[0] = (char)(0xE0 | ((c >> 12) & 0x0F));
        out[1] = (char)(0x80 | ((c >> 6) & 0x3F));
        out[2] = (char)(0x80 | (c & 0x3F));
        return 3;
    } else if (c <= 0x10FFFF) {
        out[0] = (char)(0xF0 | ((c >> 18) & 0x07));
        out[1] = (char)(0x80 | ((c >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((c >> 6) & 0x3F));
        out[3] = (char)(0x80 | (c & 0x3F));
        return 4;
    }

    out[0] = '?';
    return 1;
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

    free(buffer);
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

                char utf8[5] = {0};
                int  len = utf8_encode(buffer->actual[i].c, utf8);
                fwrite(utf8, 1, len, stdout);

                buffer->old[i] = buffer->actual[i];
            }
        }
    }

    printf("\033[0m");
    fflush(stdout);
}

int buffer_resize(Buffer* buffer) {
    if (!buffer) return 0;

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (buffer->window_cols == w.ws_col && buffer->window_rows == w.ws_row)
            return 1;

        buffer->window_rows = w.ws_row;
        buffer->window_cols = w.ws_col;

        if (buffer->old) free(buffer->old);
        if (buffer->actual) free(buffer->actual);

        size_t size = buffer->window_cols * buffer->window_rows;
        buffer->size = size;
        buffer->actual = malloc(size * sizeof(Cell));
        if (!buffer->actual) {
            errlog(ERR_MALLOC_NULL, "buffer:buffer_resize:actual");
            return 0;
        }
        memset(buffer->actual, 0, size * sizeof(Cell));

        buffer->old = malloc(size * sizeof(Cell));
        if (!buffer->old) {
            errlog(ERR_MALLOC_NULL, "buffer:buffer_resize:old");
            free(buffer->actual);
            return 0;
        }
        memset(buffer->old, 0, size * sizeof(Cell));
        return 1;
    }

    return 0;
}

void buffer_set_char(Buffer* buffer, Vec v, u32 ch) {
    buffer_set_cell(buffer, v, ch, COLOR_DEFAULT, COLOR_DEFAULT, STYLE_NONE);
}

void buffer_set_cell(Buffer* buffer, Vec v, u32 ch, i32 tg, i32 bg, u8 fl) {
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

    const char* p = line;

    while (*p && v.x < buffer->window_cols) {
        u32 ch = utf8_decode(&p);
        buffer_set_char(buffer, v, ch);
        v.x++;
    }
}

void buffer_append_line_styled(Buffer* buffer, Vec v, const char* line, i32 tg,
                               i32 bg, u8 fl) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    const char* p = line;

    while (*p && v.x < buffer->window_cols) {
        u32 ch = utf8_decode(&p);
        buffer_set_cell(buffer, v, ch, tg, bg, fl);
        v.x++;
    }
}

size_t buffer_append_line_offset(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return 0;

    const char* p = line;

    size_t offset = 0;
    while (*p) {
        if (v.x >= buffer->window_cols) {
            v.x = 0;
            v.y++;
            offset++;
        }

        buffer_set_char(buffer, v, utf8_decode(&p));

        v.x++;
    }

    return offset;
}

void buffer_append_vertical_line(Buffer* buffer, Vec v, const char* line) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    const char* p = line;

    while (*p && v.y < buffer->window_rows) {
        u32 ch = utf8_decode(&p);
        buffer_set_char(buffer, v, ch);
        v.y++;
    }
}

void buffer_append_line_vertical_styled(Buffer* buffer, Vec v, const char* line,
                                        i32 tg, i32 bg, u8 fl) {
    if (!line || v.x >= buffer->window_cols || v.y >= buffer->window_rows)
        return;

    const char* p = line;

    while (*p && v.y < buffer->window_rows) {
        u32 ch = utf8_decode(&p);
        buffer_set_cell(buffer, v, tg, bg, fl, ch);
        v.y++;
    }
}

void buffer_set_range_char(Buffer* buffer, Vec range, Vec v, u32 ch) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.x > buffer->window_cols) return;

    for (size_t i = range.x; i < range.y - range.x; i++) {
        buffer_set_char(buffer, (Vec){v.x + i, v.y}, ch);
    }
}

void buffer_set_ver_range_char(Buffer* buffer, Vec range, Vec v, u32 ch) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.y > buffer->window_rows) return;

    for (size_t i = range.x; i < range.y - range.x; i++) {
        buffer_set_char(buffer, (Vec){v.x, v.y + i}, ch);
    }
}

void buffer_append_range(Buffer* buffer, Vec range, Vec v, const char* line) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.x > buffer->window_cols) return;

    u32    buf[256];
    size_t count = 0;

    const char* p = line;

    while (*p && count < 256) {
        buf[count++] = utf8_decode(&p);
    }

    if (count == 0) return;

    size_t idx = 0;

    for (size_t i = range.x; i < range.y - range.x; i++) {
        buffer_set_char(buffer, (Vec){v.x + i, v.y}, buf[idx]);
        idx++;
        if (idx >= count) idx = 0;
    }
}

void buffer_append_ver_range(Buffer* buffer, Vec range, Vec v,
                             const char* line) {
    if (v.x >= buffer->window_cols || v.y >= buffer->window_rows) return;
    if (range.y + v.y > buffer->window_rows) return;

    u32    buf[256];
    size_t count = 0;

    const char* p = line;

    while (*p && count < 256) {
        buf[count++] = utf8_decode(&p);
    }

    if (count == 0) return;

    size_t idx = 0;

    for (size_t i = range.x; i < range.y; i++) {
        buffer_set_char(buffer, (Vec){v.x, v.y + i}, buf[idx]);
        idx++;
        if (idx >= count) idx = 0;
    }
}

void buffer_clear_line(Buffer* buffer, Vec range, size_t y) {
    if (range.y >= buffer->window_cols || y >= buffer->window_rows) return;

    for (size_t i = range.x; i <= range.y; i++) {
        size_t index = to_index(buffer, (Vec){i, y});
        if (buffer->actual[index].c != 0x20) {
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

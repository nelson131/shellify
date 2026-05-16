#include "rect.h"

void draw_rect(Buffer* buffer, Rect r) {
    char* line = malloc((r.w + 1) * sizeof(char));
    if (!line) {
        errlog(ERR_MALLOC_NULL, "tui:draw_rect:line");
        return;
    }

    for (size_t i = 0; i < r.w - 1; i++) {
        line[i] = '-';
    }
    line[0] = '+';
    line[r.w - 1] = '+';
    line[r.w] = '\0';

    buffer_append_line(buffer, r.vec, line);
    buffer_append_line(buffer, (Vec){r.vec.x, r.vec.y + r.h - 1}, line);
    free(line);

    char* wall = malloc((r.h - 1) * sizeof(char));
    if (!wall) {
        errlog(ERR_MALLOC_NULL, "tui:draw_rect:wall");
        return;
    }

    for (size_t i = 0; i < r.h - 2; i++) {
        wall[i] = '|';
    }
    wall[r.h - 2] = '\0';

    buffer_append_vertical_line(buffer, (Vec){r.vec.x, r.vec.y + 1}, wall);
    buffer_append_vertical_line(buffer, (Vec){r.vec.x + r.w - 1, r.vec.y + 1},
                                wall);
    free(wall);
}

void rect_center(Rect* rect, size_t w, size_t h) {
    rect_center_x(rect, w);
    rect_center_y(rect, h);
}

void rect_center_x(Rect* rect, size_t w) {
    if (w > rect->w) {
        rect->vec.x = (w - rect->w) / 2;
    }
}

void rect_center_y(Rect* rect, size_t h) {
    if (h > rect->h) {
        rect->vec.y = (h - rect->h) / 2;
    }
}

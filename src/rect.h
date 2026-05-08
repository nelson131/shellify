#ifndef RECT_H
#define RECT_H

#include "buffer.h"
#include "vec.h"

typedef struct Rect {
    Vec    vec;
    size_t w;
    size_t h;
} Rect;

void draw_rect(Buffer* buffer, Rect r);

void rect_center(Rect* rect, size_t w, size_t h);
void rect_center_x(Rect* rect, size_t w);
void rect_center_y(Rect* rect, size_t h);

#endif

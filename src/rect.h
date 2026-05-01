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

#endif

#ifndef NEW_BUFFER_H
#define NEW_BUFFER_H

#include <stdint.h>

#include "logger.h"
#include "vec.h"

#define STYLE_NONE 0
#define STYLE_BOLD (1 << 0)
#define STYLE_UNDERLINE (1 << 1)

#define COLOR_DEFAULT -1
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

typedef int32_t i32;
typedef uint8_t u8;

typedef struct Cell {
    char c;
    i32  tg;
    i32  bg;
    u8   fl;
} Cell;

typedef struct Buffer {
    Cell* actual;
    Cell* old;

    size_t window_cols;
    size_t window_rows;
    size_t size;
} Buffer;

static size_t to_index(Buffer* buffer, Vec v);
static void   cell_reset(Cell* cell);
static int    cell_eq(Cell* c1, Cell* c2);
static void   cell_cmpl(Cell* c, i32* c_tg, i32* c_bg, u8* c_fl);

int  buffer_init(Buffer** buffer, size_t* window_cols, size_t* window_rows);
void buffer_clear(Buffer* buffer);
void buffer_destroy(Buffer* buffer);

void buffer_render(Buffer* buffer);
void buffer_render_full(Buffer* buffer);

void buffer_set_char(Buffer* buffer, Vec v, char line);
void buffer_set_cell(Buffer* buffer, Vec v, char ch, i32 tg, i32 bg, u8 fl);

void buffer_append_line(Buffer* buffer, Vec v, const char* line);
void buffer_append_line_styled(Buffer* buffer, Vec v, const char* line, i32 tg,
                               i32 bg, u8 fl);
void buffer_append_vertical_line(Buffer* buffer, Vec v, const char* line);
void buffer_append_vertical_line_styled(Buffer* buffer, Vec v, const char* line,
                                        i32 tg, i32 bg, u8 fl);

void buffer_set_range_char(Buffer* buffer, Vec range, Vec v, char ch);
void buffer_set_ver_range_char(Buffer* buffer, Vec range, Vec v, char ch);
void buffer_append_range(Buffer* buffer, Vec range, Vec v, const char* line);
void buffer_append_ver_range(Buffer* buffer, Vec range, Vec v,
                             const char* line);

void buffer_clear_line(Buffer* buffer, Vec range, size_t y);
void buffer_clear_vertical_line(Buffer* buffer, Vec range, size_t x);
void buffer_clear_square(Buffer* buffer, Vec v1, Vec v2);

#endif

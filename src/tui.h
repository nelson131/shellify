#ifndef TUI_H
#define TUI_H

#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "config.h"
#include "error_handler.h"

#define PREFIX_PLAYING " Now playing: "

typedef struct TUI {
    size_t header_top_border;
    size_t header_bottom_border;

    char* separator;
    char* line_status;
    char* song_name;
} TUI;

int  tui_init(TUI** tui, size_t* window_cols, size_t* window_rows);
void tui_clear(TUI* tui);

int create_header(TUI* tui, Buffer* buffer, Config* config);
int create_welcome(TUI* tui, Buffer* buffer, Config* config);

#endif

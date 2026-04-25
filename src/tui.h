#ifndef TUI_H
#define TUI_H

#include <stdlib.h>
#include <string.h>

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

extern TUI* tui;

int  tui_init();
void tui_clear();

int create_header();
int operate_welcome();

#endif

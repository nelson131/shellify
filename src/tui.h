#ifndef TUI_H
#define TUI_H

#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "config.h"
#include "error_handler.h"
#include "rect.h"
#include "storage.h"

#define PREFIX_PLAYING " Now playing: "

typedef struct TUI {
    size_t header_top_border;
    size_t header_bottom_border;

    char* separator;
    char* line_status;
    char* song_name;

    size_t playlist_wall;
    size_t x_songs;
    size_t y_songs;
    size_t x_playlists;
    size_t y_playlists;

    size_t selected_song_idx;
    size_t selected_playlist_idx;
} TUI;

int  tui_init(TUI** tui, size_t* window_cols, size_t* window_rows);
void tui_clear(TUI* tui);

void tui_update(TUI* tui, size_t* window_cols, size_t* window_rows);

int create_header(TUI* tui, Buffer* buffer, Config* config);
int create_welcome(TUI* tui, Buffer* buffer, Config* config);

int create_player(TUI* tui, Library* library, Buffer* buffer, Config* config);

int create_add_menu(TUI* tui, Library* library, Buffer* buffer, Config* config);

#endif

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

typedef struct TUI_InputForm {
    char** options;
    char** values;

    size_t size;
    size_t cap;
    int    selected_option;

    size_t str_len;
} TUI_InputForm;

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

    size_t selected_index;

    TUI_InputForm* input_form;
} TUI;

int  tui_init(TUI** tui, size_t* window_cols, size_t* window_rows);
void tui_clear(TUI* tui);

void tui_update(TUI* tui, size_t* window_cols, size_t* window_rows);

int create_header(TUI* tui, Buffer* buffer, Config* config, const char* mode);
int create_welcome(TUI* tui, Buffer* buffer, Config* config);

int create_player(TUI* tui, Library* library, Buffer* buffer, Config* config);

int create_add_menu(TUI* tui, Buffer* buffer, Config* config);

TUI_InputForm* create_input_form(size_t cap);
void           clear_input_form(TUI_InputForm* form);
void           put_in_form(TUI_InputForm* form, size_t idx, const char* msg);
int create_input_menu(TUI* tui, Buffer* buffer, TUI_InputForm* form, Rect rect,
                      const char* msg);

int create_add_local_menu(TUI* tui, Buffer* buffer, TUI_InputForm* form);

#endif

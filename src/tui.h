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
#define PREFIX_PLAYLING_LEN strlen(PREFIX_PLAYING)

#define UNICODE_VER_LINE "|"

typedef struct TUI_InputForm {
    char** options;
    char** values;

    size_t size;
    size_t cap;
    int    selected_option;

    size_t str_len;
} TUI_InputForm;

typedef struct TUI_ChoiceForm {
    char** options;

    size_t size;
    size_t cap;
    int    selected_option;

    size_t str_len;
} TUI_ChoiceForm;

typedef struct TUI {
    size_t header_top_border;
    size_t header_bottom_border;

    char* separator;
    char* song_name;

    size_t playlist_wall;
    size_t x_songs;
    size_t y_songs;
    size_t x_playlists;
    size_t y_playlists;

    size_t idx_plists;
    size_t idx_songs;

    TUI_InputForm*  input_form;
    TUI_ChoiceForm* choice_form;
} TUI;

int  tui_init(TUI** tui, size_t* window_cols, size_t* window_rows);
void tui_update(TUI* tui, size_t* window_cols, size_t* window_rows);
void tui_clear(TUI* tui);

// >>> tui elements and interfaces
// bosses
void make_welcome(TUI* tui, Buffer* buffer, Config* config);
void make_header(TUI* tui, Buffer* buffer, Config* config, const char* mode);
void make_player(TUI* tui, Library* library, Buffer* buffer, Config* config);
// views
void view_plists(TUI* tui, Library* library, Buffer* buffer);
void view_songs(TUI* tui, Library* library, Buffer* buffer);
// ADD song
void make_add_sn(TUI* tui, Buffer* buffer, Config* config);
void make_add_local_sn(TUI* tui, Buffer* buffer, Config* config);
// ADD playlist
void make_add_plist(TUI* tui, Buffer* buffer, Config* config);

// >>> input form handler
void create_input_form(TUI* tui, size_t cap);
void set_input_form(TUI* tui, const char* options[], size_t cap);

void clear_input_form(TUI* tui);
void put_inform(TUI_InputForm* form, size_t idx, const char* msg);

void make_input_form(TUI* tui, Buffer* buffer, Rect rect, const char* msg);

// >>> choice form handler
void create_choice_form(TUI* tui, size_t cap);
void set_choice_form(TUI* tui, const char* options[], size_t cap);

void clear_choice_form(TUI* tui);
void put_chform(TUI_ChoiceForm* form, size_t idx, const char* msg);

void make_choice_form(TUI* tui, Buffer* buffer, Rect rect, const char* msg);

#endif

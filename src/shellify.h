#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <sys/ioctl.h>
#include <termios.h>

#include "audio.h"
#include "buffer.h"
#include "config.h"
#include "input.h"
#include "logger.h"
#include "stg_handler.h"
#include "storage.h"
#include "tui.h"

typedef enum ShellifyState {
    SHELLIFY_STATE_WELCOME,
    SHELLIFY_STATE_PLAYER,
    SHELLIFY_STATE_ADD_PLAYLIST,
    SHELLIFY_STATE_ADD_SONG,
    SHELLIFY_STATE_ADD_SONG_LOCAL,
    SHELLIFY_STATE_ADD_SONG_YTDLP
} ShellifyState;

typedef enum FocusState { SHELLIFY_PLAYLISTS, SHELLIFY_SONGS } FocusState;

typedef struct Shellify {
    int           is_running;
    ShellifyState state;
    InputState    input_state;
    FocusState    focus_state;

    size_t window_cols;
    size_t window_rows;

    Config* config;

    Buffer* buffer;

    TUI* tui;

    sqlite3* db;
    Library* library;

    Audio* audio;
} Shellify;

extern Shellify* shellify;

void shellify_init();
void shellify_destroy();

void shellify_update();
void shellify_draw();
void shellify_draw_state();
void shellify_handle_input();

void shellify_stop();

#endif

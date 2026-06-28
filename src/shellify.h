#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <sys/ioctl.h>
#include <termios.h>

#include "audio.h"
#include "buffer.h"
#include "config.h"
#include "controller.h"
#include "dl_handler.h"
#include "dl_queue.h"
#include "input.h"
#include "logger.h"
#include "signal.h"
#include "storage.h"
#include "tui.h"

typedef enum ShellifyState {
    SHELLIFY_STATE_WELCOME,
    SHELLIFY_STATE_PLAYER,
    SHELLIFY_STATE_ADD_PLAYLIST,
    SHELLIFY_STATE_ADD_SONG,
    SHELLIFY_STATE_ADD_SONG_LOCAL,
    SHELLIFY_STATE_ADD_SONG_YTDLP_LINK,
    SHELLIFY_STATE_ADD_SONG_YTDLP_SEARCH,
    SHELLIFY_STATE_DASHBOARD
} ShellifyState;

typedef enum FocusState { SHELLIFY_PLAYLISTS, SHELLIFY_SONGS } FocusState;

typedef struct Shellify {
    int           is_running;
    ShellifyState state;
    InputState    input_state;
    FocusState    focus_state;
    DLState       dl_state;

    size_t window_cols;
    size_t window_rows;

    Config* config;

    Buffer* buffer;

    TUI* tui;

    Storage* stg;

    Audio* audio;
} Shellify;

extern Shellify* shellify;

void shellify_init();
void shellify_destroy();

int  shellify_screen_size();
void shellify_update();
void shellify_draw();
void shellify_draw_state();
void shellify_handle_input();

void shellify_stop();

#endif

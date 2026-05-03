#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <sys/ioctl.h>
#include <termios.h>

#include "buffer.h"
#include "config.h"
#include "file.h"
#include "input.h"
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

typedef struct Shellify {
    int           is_running;
    ShellifyState state;
    InputState    input_state;

    size_t window_cols;
    size_t window_rows;

    Config* config;

    Buffer* buffer;

    TUI* tui;

    sqlite3* db;
    Library* library;
} Shellify;

extern Shellify* shellify;

void shellify_init();
void shellify_destroy();

void shellify_update();
void shellify_draw();
void shellify_handle_input();

void shellify_stop();

#endif

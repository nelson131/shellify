#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <sys/ioctl.h>
#include <termios.h>

#include "buffer.h"
#include "config.h"
#include "input.h"
#include "storage.h"
#include "tui.h"

typedef enum ShellifyState {
    SHELLIFY_STATE_WELCOME,
    SHELLIFY_STATE_PLAYER
} ShellifyState;

typedef struct Shellify {
    int           is_running;
    ShellifyState state;

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

void shellify_draw();
void shellify_handle_input();

void shellify_stop();

#endif

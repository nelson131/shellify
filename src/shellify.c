#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "storage.h"

int      shellify_is_running = 1;
sqlite3* db = NULL;

void shellify_init() {
    if (!config_load()) {
        raise_error(ERR_CONFIG_LOAD, "something went wrong in config saving");
    }

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

    if (!buffer_init(winsize.ws_col, winsize.ws_row)) shellify_stop();
    if (!tui_init()) shellify_stop();

    db = storage_init();
    if (!db) shellify_stop();

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(ICANON | ECHO);
    term.c_iflag &= ~(IXON | ICRNL);

    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    if (!create_header()) shellify_stop();

    printf("\033[2J");
    printf("\033[H");
}

void shellify_destroy() {
    if (!config_save()) {
        raise_error(ERR_CONFIG_SAVE, "something went wrong in config saving");
    }

    buffer_destroy();
    tui_clear();
    if (!storage_close(&db)) raise_error(FAILED, "shellify:destroy:storage");

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void shellify_draw() { buffer_render(); }

void shellify_handle_input() {
    int key = input_poll();

    if (key == -1) return;

    if (key == 'q') {
        shellify_is_running = 0;
    }

    if (key == KEY_ARROW_UP) {
        buffer_set_char(10, 10, '^');
        buffer_append_line(10, 12, "kids are doing WHAAT?");
    }
}

void shellify_stop() { shellify_is_running = 0; }

#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int shellify_is_running = 1;

void shellify_init() {
    if (!config_load()) {
        raise_error(ERR_CONFIG_LOAD, "something went wrong in config saving");
    }

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    buffer_init(winsize.ws_col, winsize.ws_row);

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(ICANON | ECHO);
    term.c_iflag &= ~(IXON | ICRNL);

    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    printf("\033[2J");
    printf("\033[H");
}

void shellify_destroy() {
    if (!config_save()) {
        raise_error(ERR_CONFIG_SAVE, "something went wrong in config saving");
    }

    buffer_destroy();

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

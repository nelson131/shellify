#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void shellify_init() {
    struct winsize winsize;
    buffer_init(winsize.ws_col, winsize.ws_row);

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    term.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void shellify_destroy() {
    buffer_destroy();

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void shellify_draw() {
    printf("\033[2J");
    printf("\033[H");

    buffer_render();
}

void shellify_handle_input() {}

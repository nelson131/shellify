#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int shellify_is_running = 1;

void shellify_init() {
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

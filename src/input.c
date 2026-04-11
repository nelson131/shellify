#include "input.h"

#include <stdio.h>
#include <stdlib.h>

int input_pause() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == '\033') {
        char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A':
                    return KEY_ARROW_UP;
                case 'B':
                    return KEY_ARROW_DOWN;
                case 'C':
                    return KEY_ARROW_RIGHT;
                case 'D':
                    return KEY_ARROW_LEFT;
            }
        }

        return KEY_ESC;
    }

    return c;
}

int input_poll() {
    int old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    int c = input_pause();

    fcntl(STDIN_FILENO, F_SETFL, old_flags);
    return c;
}

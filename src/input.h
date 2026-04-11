#ifndef INPUT_H
#define INPUT_H

#include <fcntl.h>
#include <unistd.h>

int input_pause();
int input_poll();

typedef enum KeyCode {
    KEY_ARROW_UP = 1000,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,
    KEY_ESC
} KeyCode;

#endif

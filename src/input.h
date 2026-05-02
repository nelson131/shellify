#ifndef INPUT_H
#define INPUT_H

#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "tui.h"

int input_pause();
int input_poll();

int handle_input_form(int key, TUI_InputForm* form, Config* config);

typedef enum KeyCode {
    KEY_ARROW_UP = 1000,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,
    KEY_ESC,
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8
} KeyCode;

#endif

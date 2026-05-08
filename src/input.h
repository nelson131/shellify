#ifndef INPUT_H
#define INPUT_H

#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "tui.h"

typedef enum InputState {
    INPUT_STATE_NONE,
    INPUT_STATE_ADD,
    INPUT_STATE_REMOVE
} InputState;

typedef enum KeyCode {
    KEY_ARROW_UP = 1000,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,
    KEY_ESC = 27,
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8
} KeyCode;

int input_pause();
int input_poll();

int handle_input_form(int key, TUI_InputForm* form, Config* config);
int handle_choice_form(int key, TUI_ChoiceForm* form, Config* config);

const char* instate_char(InputState input_state);

#endif

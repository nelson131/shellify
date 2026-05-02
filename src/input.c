#include "input.h"

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

// used for input forms
// returning 0 as default
// returning 1 as confirmation of send of input form result
int handle_input_form(int key, TUI_InputForm* form, Config* config) {
    if (!form) return 0;

    char* cur_value = NULL;
    cur_value = form->values[form->selected_option];

    if (key == KEY_ARROW_UP && form->selected_option - 1 >= 0) {
        form->selected_option -= 1;
    } else if (key == KEY_ARROW_DOWN &&
               form->selected_option + 1 <= form->size) {
        form->selected_option += 1;
    } else if (key == KEY_BACKSPACE) {
        size_t len = strlen(cur_value);
        if (len > 0) cur_value[len - 1] = '\0';
    } else if (key == config->keys.select) {
        for (size_t i = 0; i < form->size; i++) {
            size_t len = strlen(form->values[i]);
            if (!len) return 0;
        }
        return 1;
    } else if (key >= 32 && key <= 126) {
        size_t len = strlen(cur_value);
        if (len < form->str_len) {
            cur_value[len] = (char)key;
            cur_value[len + 1] = '\0';
        }
    }

    return 0;
}

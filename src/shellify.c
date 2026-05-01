#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "storage.h"
#include "tui.h"

Shellify* shellify = NULL;

void shellify_init() {
    shellify = malloc(sizeof(Shellify));
    if (!shellify) shellify_stop();

    shellify->is_running = 1;
    shellify->db = NULL;
    shellify->state = SHELLIFY_STATE_WELCOME;

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    shellify->window_cols = winsize.ws_col;
    shellify->window_rows = winsize.ws_row;

    if (!config_load(&shellify->config)) shellify_stop();

    if (!buffer_init(&shellify->buffer, &shellify->window_cols,
                     &shellify->window_rows))
        shellify_stop();

    if (!tui_init(&shellify->tui, &shellify->window_cols,
                  &shellify->window_rows))
        shellify_stop();

    if (!storage_init(&shellify->db)) shellify_stop();

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(ICANON | ECHO);
    term.c_iflag &= ~(IXON | ICRNL);

    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    printf("\033[2J");
    printf("\033[H");

    if (!create_header(shellify->tui, shellify->buffer, shellify->config))
        shellify_stop();

    if (!create_welcome(shellify->tui, shellify->buffer, shellify->config))
        shellify_stop();
}

void shellify_destroy() {
    if (!config_save(shellify->config)) {
        raise_error(ERR_CONFIG_SAVE, "something went wrong in config saving");
    }
    free(shellify->config);

    buffer_destroy(shellify->buffer);
    free(shellify->buffer);

    tui_clear(shellify->tui);
    free(shellify->tui);

    if (!storage_close(&shellify->db, &shellify->library))
        raise_error(FAILED, "shellify:destroy:storage");

    free(shellify);

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void shellify_update() {}

void shellify_draw() { buffer_render(shellify->buffer); }

void shellify_handle_input() {
    int key = input_poll();
    if (key == -1) return;

    if (key == shellify->config->keys.quit) {
        shellify->is_running = 0;
        return;
    }

    if (shellify->state == SHELLIFY_STATE_WELCOME) {
        if (key == shellify->config->keys.select) {
            shellify->state = SHELLIFY_STATE_PLAYER;
            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config);

            if (!storage_load(shellify->db, &shellify->library)) {
                raise_error(FAILED, "shellify:storage_load");
                shellify_stop();
                return;
            }

            create_player(shellify->tui, shellify->library, shellify->buffer,
                          shellify->config);

            return;
        }
    }

    if (shellify->state == SHELLIFY_STATE_PLAYER) {
        if (key == KEY_ARROW_UP) {
            buffer_set_char(shellify->buffer, (Vec){10, 10}, '^');
            buffer_append_line(shellify->buffer, (Vec){10, 12},
                               "the pain, it comes in waves");
            return;
        }

        if (key == shellify->config->keys.add) {
            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config);
            return;
        }
    }
}

void shellify_stop() { shellify->is_running = 0; }

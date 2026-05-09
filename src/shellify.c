#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "input.h"
#include "storage.h"
#include "tui.h"

Shellify* shellify = NULL;

void shellify_init() {
    printf("\033[?25l");
    fflush(stdout);

    shellify = malloc(sizeof(Shellify));
    if (!shellify) shellify_stop();

    shellify->is_running = 1;
    shellify->db = NULL;
    shellify->state = SHELLIFY_STATE_WELCOME;
    shellify->input_state = INPUT_STATE_NONE;

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
}

void shellify_destroy() {
    printf("\033[?25h");
    fflush(stdout);

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

void shellify_draw() {
    shellify_draw_state();
    buffer_render(shellify->buffer);
    fflush(stdout);
}

void shellify_draw_state() {
    buffer_clear(shellify->buffer);

    if (shellify->state != SHELLIFY_STATE_WELCOME) {
        make_header(shellify->tui, shellify->buffer, shellify->config,
                    instate_char(shellify->input_state));
    }

    switch (shellify->state) {
        case SHELLIFY_STATE_WELCOME:
            make_welcome(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_PLAYER:
            make_player(shellify->tui, shellify->library, shellify->buffer,
                        shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG:
            make_add_sn(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP:
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            break;
        default:
            break;
    }
}

void shellify_handle_input() {
    int key = input_poll();
    if (key == -1) return;

    if (key == shellify->config->keys.quit) {
        shellify_stop();
        return;
    }

    switch (shellify->state) {
        case SHELLIFY_STATE_WELCOME:
            if (key == shellify->config->keys.select) {
                if (!storage_load(shellify->db, &shellify->library)) {
                    raise_error(FAILED, "shellify:load_storage");
                    shellify_stop();
                    break;
                }
                shellify->state = SHELLIFY_STATE_PLAYER;
            }
            break;
        case SHELLIFY_STATE_PLAYER:
            switch (shellify->input_state) {
                case INPUT_STATE_NONE:
                    if (key == shellify->config->keys.add) {
                        shellify->input_state = INPUT_STATE_ADD;
                    } else if (key == shellify->config->keys.remove) {
                        shellify->input_state = INPUT_STATE_REMOVE;
                    }
                    break;
                case INPUT_STATE_ADD:
                    if (key == shellify->config->keys.song) {
                        shellify->state = SHELLIFY_STATE_ADD_SONG;
                        shellify->input_state = INPUT_STATE_NONE;
                        return;
                    } else if (key == shellify->config->keys.playlist) {
                        shellify->state = SHELLIFY_STATE_ADD_PLAYLIST;
                        shellify->input_state = INPUT_STATE_NONE;
                        return;
                    } else if (key == KEY_ESC) {
                        shellify->input_state = INPUT_STATE_NONE;
                    }
                    break;
                case INPUT_STATE_REMOVE:
                    break;
                default:
                    shellify->input_state = INPUT_STATE_NONE;
                    break;
            }
            break;
        case SHELLIFY_STATE_ADD_SONG:
            if (key == KEY_ARROW_LEFT) {
                shellify->state = SHELLIFY_STATE_PLAYER;
                return;
            }
            int idx = handle_choice_form(key, shellify->tui->choice_form,
                                         shellify->config);
            if (idx >= 0) {
                switch (idx) {
                    case 0:
                        break;
                    case 1:
                        break;
                    default:
                        break;
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP:
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            break;
        default:
            shellify->state = SHELLIFY_STATE_PLAYER;
            break;
    }
}

void shellify_stop() { shellify->is_running = 0; }

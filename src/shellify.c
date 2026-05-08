#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
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

    make_header(shellify->tui, shellify->buffer, shellify->config,
                instate_char(shellify->input_state));
    make_welcome(shellify->tui, shellify->buffer, shellify->config);
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

void shellify_draw() { buffer_render(shellify->buffer); }

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
                buffer_clear(shellify->buffer);
                make_header(shellify->tui, shellify->buffer, shellify->config,
                            instate_char(shellify->input_state));
                make_player(shellify->tui, shellify->library, shellify->buffer,
                            shellify->config);
            } else if (key == shellify->config->keys.quit) {
                shellify_stop();
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

                    buffer_clear(shellify->buffer);
                    make_header(shellify->tui, shellify->buffer,
                                shellify->config,
                                instate_char(shellify->input_state));
                    make_player(shellify->tui, shellify->library,
                                shellify->buffer, shellify->config);
                    break;
                case INPUT_STATE_ADD:
                    if (key == shellify->config->keys.song) {
                        shellify->state = SHELLIFY_STATE_ADD_SONG;
                        shellify->input_state = INPUT_STATE_NONE;
                        shellify->tui->choice_form->selected_option = 0;

                        buffer_clear(shellify->buffer);
                        make_header(shellify->tui, shellify->buffer,
                                    shellify->config,
                                    instate_char(shellify->input_state));
                        make_add_sn(shellify->tui, shellify->buffer,
                                    shellify->config);
                        return;
                    } else if (key == shellify->config->keys.playlist) {
                        shellify->state = SHELLIFY_STATE_ADD_PLAYLIST;
                        shellify->input_state = INPUT_STATE_NONE;
                        shellify->tui->choice_form->selected_option = 0;

                        buffer_clear(shellify->buffer);
                        make_header(shellify->tui, shellify->buffer,
                                    shellify->config,
                                    instate_char(shellify->input_state));
                        make_add_sn(shellify->tui, shellify->buffer,
                                    shellify->config);
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
                buffer_clear(shellify->buffer);
                make_header(shellify->tui, shellify->buffer, shellify->config,
                            instate_char(shellify->input_state));
                make_player(shellify->tui, shellify->library, shellify->buffer,
                            shellify->config);
                return;
            } else if (key == KEY_ARROW_UP) {
                shellify->tui->selected_index = 0;
            } else if (key == KEY_ARROW_DOWN) {
                shellify->tui->selected_index = 1;
            } else if (key == shellify->config->keys.select) {
                if (shellify->tui->selected_index == 0) {
                    shellify->state = SHELLIFY_STATE_ADD_SONG_LOCAL;
                    shellify->tui->input_form = create_input_form(5);
                    buffer_clear(shellify->buffer);
                    create_header(shellify->tui, shellify->buffer,
                                  shellify->config,
                                  instate_char(shellify->input_state));
                    create_add_local_menu(shellify->tui, shellify->buffer,
                                          shellify->tui->input_form);
                    return;
                } else {
                    shellify->state = SHELLIFY_STATE_ADD_SONG_YTDLP;
                }
            }

            create_header(shellify->tui, shellify->buffer, shellify->config,
                          instate_char(shellify->input_state));
            create_add_menu(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            if (!shellify->tui->input_form) break;

            if (key == KEY_ARROW_LEFT) {
                clear_input_form(shellify->tui->input_form);
                shellify->tui->input_form = NULL;

                shellify->state = SHELLIFY_STATE_ADD_SONG;
                buffer_clear(shellify->buffer);
                create_header(shellify->tui, shellify->buffer, shellify->config,
                              instate_char(shellify->input_state));
                create_add_menu(shellify->tui, shellify->buffer,
                                shellify->config);
                return;
            } else if (handle_input_form(key, shellify->tui->input_form,
                                         shellify->config)) {
                shellify->state = SHELLIFY_STATE_PLAYER;
                TUI_InputForm* form = shellify->tui->input_form;
                Song*          song = storage_create_song(
                    shellify->library, 0, form->options[1], form->options[2],
                    form->options[3], form->options[4],
                    file_get_duration_sec(form->options[1], shellify->config),
                    get_time());
                storage_add_song(shellify->db, shellify->library, song);

                clear_input_form(shellify->tui->input_form);
                shellify->tui->input_form = NULL;

                buffer_clear(shellify->buffer);
                create_header(shellify->tui, shellify->buffer, shellify->config,
                              instate_char(shellify->input_state));
                create_player(shellify->tui, shellify->library,
                              shellify->buffer, shellify->config);
                return;
            }

            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config,
                          instate_char(shellify->input_state));
            create_add_local_menu(shellify->tui, shellify->buffer,
                                  shellify->tui->input_form);
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP:
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            if (!shellify->tui->input_form) break;

            if (key == KEY_ARROW_LEFT) {
                clear_input_form(shellify->tui->input_form);
                shellify->tui->input_form = NULL;

                shellify->state = SHELLIFY_STATE_PLAYER;
                buffer_clear(shellify->buffer);
                create_header(shellify->tui, shellify->buffer, shellify->config,
                              instate_char(shellify->input_state));
                create_player(shellify->tui, shellify->library,
                              shellify->buffer, shellify->config);
                return;
            } else if (handle_input_form(key, shellify->tui->input_form,
                                         shellify->config)) {
                shellify->state = SHELLIFY_STATE_PLAYER;
            }

            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config,
                          instate_char(shellify->input_state));
            create_add_playlist_menu(shellify->tui, shellify->buffer,
                                     shellify->config,
                                     shellify->tui->input_form);
            break;
        default:
            shellify->state = SHELLIFY_STATE_PLAYER;
            break;
    }
}

void shellify_stop() { shellify->is_running = 0; }

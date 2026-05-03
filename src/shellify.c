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
        shellify->is_running = 0;
        return;
    }

    // WELCOME TUI
    if (shellify->state == SHELLIFY_STATE_WELCOME) {
        // moving to main player menu
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

    // PLAYER TUI: main menu of shellify, its for playlists and songs
    if (shellify->state == SHELLIFY_STATE_PLAYER) {
        // just test
        if (key == KEY_ARROW_UP) {
            buffer_set_char(shellify->buffer, (Vec){10, 10}, '^');
            buffer_append_line(shellify->buffer, (Vec){10, 12},
                               "the pain, it comes in waves");
            return;
        }

        // open the "add new song" menu
        if (key == shellify->config->keys.add) {
            shellify->state = SHELLIFY_STATE_ADD_MENU;
            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config);
            create_add_menu(shellify->tui, shellify->buffer, shellify->config);
            return;
        }
    }

    // ADD NEW SONG MENU
    if (shellify->state == SHELLIFY_STATE_ADD_MENU) {
        if (key == KEY_ARROW_LEFT) {
            // returning to the player menu
            shellify->state = SHELLIFY_STATE_PLAYER;
            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config);
            create_player(shellify->tui, shellify->library, shellify->buffer,
                          shellify->config);
            return;
        } else if (key == KEY_ARROW_UP) {
            // choice: from local files
            shellify->tui->selected_index = 0;
        } else if (key == KEY_ARROW_DOWN) {
            // choice: yt-dlp
            shellify->tui->selected_index = 1;
        } else if (key == shellify->config->keys.select) {
            if (shellify->tui->selected_index == 0) {
                // local files
                shellify->state = SHELLIFY_STATE_ADD_MENU_LOCAL;

                shellify->tui->input_form = create_input_form(5);
                buffer_clear(shellify->buffer);
                create_header(shellify->tui, shellify->buffer,
                              shellify->config);
                create_add_local_menu(shellify->tui, shellify->buffer,
                                      shellify->tui->input_form);
                return;
            } else {
                // yt-dlp downloading
                shellify->state = SHELLIFY_STATE_ADD_MENU_YTDLP;
                return;
            }
        }

        create_header(shellify->tui, shellify->buffer, shellify->config);
        create_add_menu(shellify->tui, shellify->buffer, shellify->config);
        return;
    }

    if (shellify->state == SHELLIFY_STATE_ADD_MENU_LOCAL) {
        if (!shellify->tui->input_form) return;

        if (key == KEY_ARROW_LEFT) {
            // returning to the add_menu
            clear_input_form(shellify->tui->input_form);
            shellify->tui->input_form = NULL;

            shellify->state = SHELLIFY_STATE_ADD_MENU;
            buffer_clear(shellify->buffer);
            create_header(shellify->tui, shellify->buffer, shellify->config);
            create_add_menu(shellify->tui, shellify->buffer, shellify->config);
            return;
        }

        // select button pressed: saving the song
        if (handle_input_form(key, shellify->tui->input_form,
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
            create_header(shellify->tui, shellify->buffer, shellify->config);
            create_player(shellify->tui, shellify->library, shellify->buffer,
                          shellify->config);
            return;
        }

        buffer_clear(shellify->buffer);
        create_header(shellify->tui, shellify->buffer, shellify->config);
        create_add_local_menu(shellify->tui, shellify->buffer,
                              shellify->tui->input_form);
        return;
    }
}

void shellify_stop() { shellify->is_running = 0; }

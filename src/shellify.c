#include "shellify.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "audio.h"
#include "buffer.h"
#include "input.h"
#include "logger.h"
#include "storage.h"
#include "tui.h"

Shellify* shellify = NULL;

void shellify_init() {
    printf("\033[?25l");
    fflush(stdout);

    logger_init("shellify.log");

    shellify = malloc(sizeof(Shellify));
    if (!shellify) {
        errlog(ERR_MALLOC_NULL, "shellify:init:shellify");
        shellify_stop();
    }

    shellify->is_running = 1;
    shellify->state = SHELLIFY_STATE_WELCOME;
    shellify->input_state = INPUT_STATE_NONE;
    shellify->focus_state = SHELLIFY_PLAYLISTS;

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    shellify->window_cols = winsize.ws_col;
    shellify->window_rows = winsize.ws_row;

    if (!config_load(&shellify->config)) {
        errlog(FAILED, "shellify:init:config_load");
        shellify_stop();
    }
    slog(INFO, "-----------");
    alog(INFO, shellify->config->general.version, "SHELLIFY launched");

    if (!buffer_init(&shellify->buffer, &shellify->window_cols,
                     &shellify->window_rows)) {
        errlog(FAILED, "shellify:init:buffer_load");
        shellify_stop();
    }

    if (!tui_init(&shellify->tui, &shellify->window_cols,
                  &shellify->window_rows)) {
        errlog(FAILED, "shellify:init:tui_init");
        shellify_stop();
    }

    shellify->stg = stg_init();
    if (!shellify->stg) {
        errlog(FAILED, "shellify:init:storage:init");
        shellify_stop();
    }

    audio_init(&shellify->audio);

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(ICANON | ECHO);
    term.c_iflag &= ~(IXON | ICRNL);

    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    printf("\033[2J");
    printf("\033[H");

    slog(INFO, "shellify has been initialized successfully.");
}

void shellify_destroy() {
    printf("\033[?25h");
    fflush(stdout);

    if (!config_save(shellify->config)) {
        errlog(ERR_CONFIG_SAVE, "something went wrong in config saving");
    }

    free(shellify->config);

    buffer_destroy(shellify->buffer);
    free(shellify->buffer);

    tui_clear(shellify->tui);
    free(shellify->tui);

    stg_close(shellify->stg);
    free(shellify->stg);

    audio_close(&shellify->audio);

    free(shellify);

    logger_close();

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    slog(INFO, "shellify has been closed successfully.");
}

void shellify_update() { tui_sync(shellify->tui, shellify->stg); }

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
            make_player(shellify->tui, shellify->stg, shellify->buffer,
                        shellify->config, shellify->focus_state);
            break;
        case SHELLIFY_STATE_ADD_SONG:
            make_add_sn(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            make_add_local_sn(shellify->tui, shellify->buffer,
                              shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP:
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            make_add_plist(shellify->tui, shellify->buffer, shellify->config);
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
                if (!stg_load(shellify->stg)) {
                    errlog(FAILED, "shellify:load_storage");
                    shellify_stop();
                    break;
                }
                shellify->state = SHELLIFY_STATE_PLAYER;
            }
            break;
        case SHELLIFY_STATE_PLAYER:
            if (key == KEY_ARROW_LEFT &&
                shellify->focus_state != SHELLIFY_PLAYLISTS) {
                shellify->focus_state = SHELLIFY_PLAYLISTS;
            } else if (key == KEY_ARROW_RIGHT &&
                       shellify->focus_state != SHELLIFY_SONGS) {
                shellify->focus_state = SHELLIFY_SONGS;
            }

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
                    if (key == shellify->config->keys.song) {
                        shellify->input_state = INPUT_STATE_NONE;
                        return;
                    } else if (key == shellify->config->keys.playlist) {
                        shellify->input_state = INPUT_STATE_NONE;
                        rem_plist(shellify->tui, shellify->stg);
                        return;
                    } else if (key == KEY_ESC) {
                        shellify->input_state = INPUT_STATE_NONE;
                    }
                    break;
                default:
                    shellify->input_state = INPUT_STATE_NONE;
                    break;
            }
            size_t* index = &shellify->tui->idx_songs;
            size_t  max = 0;
            if (shellify->focus_state == SHELLIFY_PLAYLISTS) {
                index = &shellify->tui->idx_plists;
                max = shellify->stg->lib->playlist_count;
            } else {
                index = &shellify->tui->idx_songs;
                max = shellify->stg->lib->playlists[shellify->tui->idx_plists]
                          ->song_count;
            }

            if (handle_player(key, index, max, shellify->config) >= 0) {
                if (shellify->focus_state == SHELLIFY_SONGS) {
                    handle_audio(shellify->tui, shellify->stg, shellify->audio);
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG:
            if (key == KEY_ARROW_LEFT) {
                shellify->state = SHELLIFY_STATE_PLAYER;
                clear_choice_form(shellify->tui);
                return;
            }
            int idx = handle_choice_form(key, shellify->tui->choice_form,
                                         shellify->config);
            if (idx >= 0) {
                switch (idx) {
                    case 0:
                        shellify->state = SHELLIFY_STATE_ADD_SONG_LOCAL;
                        break;
                    case 1:
                        break;
                    default:
                        break;
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            if (key == KEY_ARROW_LEFT) {
                shellify->state = SHELLIFY_STATE_ADD_SONG;
                clear_input_form(shellify->tui);
                return;
            } else if (handle_input_form(key, shellify->tui->input_form,
                                         shellify->config)) {
                add_song(shellify->tui, shellify->stg);
                clear_input_form(shellify->tui);
                shellify->state = SHELLIFY_STATE_PLAYER;
            }

            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP:
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            if (key == KEY_ARROW_LEFT) {
                shellify->state = SHELLIFY_STATE_PLAYER;
                clear_input_form(shellify->tui);
                shellify->tui->input_form = NULL;
            } else if (handle_input_form(key, shellify->tui->input_form,
                                         shellify->config)) {
            }
            break;
        default:
            shellify->state = SHELLIFY_STATE_PLAYER;
            break;
    }
}

void shellify_stop() {
    slog(WARNING, "shellify is forced to close.");
    shellify->is_running = 0;
}

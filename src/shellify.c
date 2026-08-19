#include "shellify.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "audio.h"
#include "buffer.h"
#include "controller.h"
#include "dl_queue.h"
#include "input.h"
#include "library.h"
#include "logger.h"
#include "storage.h"
#include "tui.h"

Shellify* shellify = NULL;

void shellify_init() {
    printf("\033[?25l");
    fflush(stdout);
    setlocale(LC_ALL, "");

    shellify = malloc(sizeof(Shellify));
    if (!shellify) {
        shellify_stop();
    }

    shellify->is_running = 1;
    shellify->state = SHELLIFY_STATE_WELCOME;
    shellify->input_state = INPUT_STATE_NONE;
    shellify->focus_state = SHELLIFY_PLAYLISTS;
    shellify->dl_state = DLSTATE_FREE;
    shellify->audio = NULL;

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    shellify->window_cols = winsize.ws_col;
    shellify->window_rows = winsize.ws_row;

    init_signals();

    if (!config_load(&shellify->config)) {
        errlog(FAILED, "shellify:init:config_load");
        shellify_stop();
        exit(1);
    }
    logger_init("shellify.log", shellify->config->general.logging);

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
    audio_volume(shellify->audio, shellify->config);

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
    tui_clear(shellify->tui);
    dlh_save_stg(shellify->stg);
    stg_close(shellify->stg);
    audio_close(&shellify->audio);

    free(shellify);

    logger_close();

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    slog(INFO, "shellify has been closed successfully.");
}

int shellify_screen_size() {
    if (window_resized) {
        window_resized = 0;
        if (buffer_resize(shellify->buffer)) {
            buffer_clear(shellify->buffer);
            tui_update(shellify->tui, &shellify->buffer->window_cols,
                       &shellify->buffer->window_rows);
            tui_up_sep(shellify->tui, &shellify->buffer->window_cols);
            return 0;
        }
    }

    return 1;
}

void shellify_update() {
    tui_sync(shellify->tui, shellify->stg);
    if (audio_is_ended(shellify->audio)) {
        handle_next(shellify->tui, shellify->stg, shellify->audio,
                    shellify->config);
        tui_up_progress_bar(shellify->tui);
    }

    dlh_run(shellify->tui, shellify->stg, shellify->audio, &shellify->dl_state,
            shellify->config);
}

void shellify_draw() {
    shellify_draw_state();
    buffer_render(shellify->buffer);
    fflush(stdout);
}

void shellify_draw_state() {
    buffer_clear(shellify->buffer);

    if (shellify->state != SHELLIFY_STATE_WELCOME) {
        make_header(shellify->tui, shellify->stg, shellify->buffer,
                    shellify->audio, shellify->config,
                    instate_char(shellify->input_state));
    }

    switch (shellify->state) {
        case SHELLIFY_STATE_WELCOME:
            make_welcome(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_PLAYER:
            make_player(shellify->tui, shellify->stg, shellify->buffer,
                        shellify->audio, shellify->config,
                        shellify->focus_state);
            break;
        case SHELLIFY_STATE_ADD_SONG:
            make_add_sn(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            make_add_local_sn(shellify->tui, shellify->buffer,
                              shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP_LINK:
            make_add_ytdlp_sn_link(shellify->tui, shellify->buffer,
                                   shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP_SEARCH:
            make_add_ytdlp_sn_search(shellify->tui, shellify->stg,
                                     shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            make_add_plist(shellify->tui, shellify->buffer, shellify->config);
            break;
        case SHELLIFY_STATE_ADD_SONG_IMPORT:
            make_add_import_sn(shellify->tui, shellify->stg, shellify->buffer,
                               shellify->config);
            break;
        case SHELLIFY_STATE_DASHBOARD:
            make_dashboard(shellify->tui, shellify->stg, shellify->buffer,
                           shellify->config);
            break;
        case SHELLIFY_STATE_UPDATE_SONG:
            make_update_sn(shellify->tui, shellify->stg, shellify->buffer);
        default:
            break;
    }
}

void shellify_handle_input() {
    int key = input_poll();
    if (key == -1) return;

    if (key == shellify->config->keys.quit &&
        shellify->input_state != INPUT_STATE_WRITING) {
        shellify_stop();
        return;
    }

    if (key == KEY_ESC && shellify->input_state != INPUT_STATE_NONE) {
        shellify->input_state = INPUT_STATE_NONE;
        if (shellify->state == SHELLIFY_STATE_ADD_SONG_YTDLP_SEARCH) {
            search_run(shellify->stg->sr_core,
                       shellify->tui->search_form->query_line);
        } else if (shellify->state == SHELLIFY_STATE_ADD_SONG_IMPORT) {
            if (!shellify->tui->search_form) return;
            if (shellify->stg->import_data) {
                clear_import_data(&shellify->stg->import_data);
            }

            shellify->stg->import_data =
                get_import_data(shellify->tui->search_form->query_line);
            if (!shellify->stg->import_data) return;

            char* query = build_query_import(
                &shellify->stg->import_data
                     ->entries[shellify->stg->import_data->idx]);
            search_run(shellify->stg->sr_core, query);
            free(query);
        }
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
                if (shellify->stg->lib->playlist_count != 0) {
                    Playlist* plist =
                        shellify->stg->lib
                            ->playlists[shellify->tui->idx_plists];
                    if (plist && plist->song_count != 0) {
                        shellify->focus_state = SHELLIFY_SONGS;
                    }
                }
            }

            switch (shellify->input_state) {
                case INPUT_STATE_NONE:
                    if (key == shellify->config->keys.skip_f) {
                        audio_skip(shellify->audio,
                                   shellify->config->player.skip_time);
                    } else if (key == shellify->config->keys.skip_b) {
                        audio_skip(shellify->audio,
                                   -shellify->config->player.skip_time);
                    } else if (key == shellify->config->keys.add) {
                        shellify->input_state = INPUT_STATE_ADD;
                    } else if (key == shellify->config->keys.remove) {
                        shellify->input_state = INPUT_STATE_REMOVE;
                    } else if (key == shellify->config->keys.shuffle) {
                        shellify->config->player.shuffle =
                            !shellify->config->player.shuffle;
                    } else if (key == shellify->config->keys.edit) {
                        shellify->input_state = INPUT_STATE_UPDATE;
                    } else if (key == shellify->config->keys.dashboard) {
                        shellify->state = SHELLIFY_STATE_DASHBOARD;
                    } else if (key == shellify->config->keys.move) {
                        shellify->input_state = INPUT_STATE_MOVE;
                    } else {
                        handle_volume(key, shellify->audio, shellify->config);
                    }
                    break;
                case INPUT_STATE_ADD:
                    if (key == shellify->config->keys.song) {
                        shellify->state = SHELLIFY_STATE_ADD_SONG;
                        shellify->input_state = INPUT_STATE_WRITING;
                        return;
                    } else if (key == shellify->config->keys.playlist) {
                        shellify->state = SHELLIFY_STATE_ADD_PLAYLIST;
                        shellify->input_state = INPUT_STATE_WRITING;
                        return;
                    }
                    break;
                case INPUT_STATE_REMOVE:
                    if (key == shellify->config->keys.song) {
                        shellify->input_state = INPUT_STATE_NONE;
                        rem_song(shellify->tui, shellify->stg, shellify->audio);
                    } else if (key == shellify->config->keys.playlist) {
                        shellify->input_state = INPUT_STATE_NONE;
                        rem_plist(shellify->tui, shellify->stg);
                    } else if (key == shellify->config->keys.super) {
                        shellify->input_state = INPUT_STATE_NONE;
                        rem_song_abs(shellify->tui, shellify->stg,
                                     shellify->audio);
                    }
                    break;
                case INPUT_STATE_UPDATE:
                    if (key == shellify->config->keys.song) {
                        shellify->state = SHELLIFY_STATE_UPDATE_SONG;
                        shellify->input_state = INPUT_STATE_WRITING;
                        return;
                    } else if (key == shellify->config->keys.playlist) {
                        return;
                    }
                case INPUT_STATE_MOVE:
                    handle_move(key, shellify->tui, shellify->stg);
                    buffer_clear(shellify->buffer);
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

            handle_player(key, index, max, shellify->config);

            if (shellify->focus_state == SHELLIFY_SONGS) {
                if (key == shellify->config->keys.select ||
                    key == shellify->config->keys.pause) {
                    handle_audio(key, shellify->tui, shellify->stg,
                                 shellify->audio, shellify->config);
                }
            }

            break;
        case SHELLIFY_STATE_ADD_SONG:
            if (key == KEY_ARROW_LEFT) {
                shellify->state = SHELLIFY_STATE_PLAYER;
                clear_choice_form(shellify->tui->choice_form);
                return;
            }
            int idx = handle_choice_form(key, shellify->tui->choice_form,
                                         shellify->config);
            if (idx >= 0) {
                switch (idx) {
                    case 0:
                        shellify->state = SHELLIFY_STATE_ADD_SONG_LOCAL;
                        shellify->input_state = INPUT_STATE_WRITING;
                        break;
                    case 1:
                        shellify->state = SHELLIFY_STATE_ADD_SONG_YTDLP_LINK;
                        shellify->input_state = INPUT_STATE_WRITING;
                        break;
                    case 2:
                        shellify->state = SHELLIFY_STATE_ADD_SONG_YTDLP_SEARCH;
                        shellify->input_state = INPUT_STATE_WRITING;
                        break;
                    case 3:
                        shellify->state = SHELLIFY_STATE_ADD_SONG_IMPORT;
                        shellify->input_state = INPUT_STATE_WRITING;
                        break;
                    default:
                        break;
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG_LOCAL:
            if (shellify->input_state == INPUT_STATE_WRITING) {
                if (handle_input_form(key, shellify->tui->input_form,
                                      shellify->config)) {
                    add_song_tui(shellify->tui, shellify->stg);
                    clear_input_form(shellify->tui);
                    shellify->state = SHELLIFY_STATE_PLAYER;
                }
            } else {
                if (key == shellify->config->keys.select) {
                    shellify->input_state = INPUT_STATE_WRITING;
                } else if (key == KEY_ARROW_LEFT) {
                    shellify->state = SHELLIFY_STATE_ADD_SONG;
                    clear_input_form(shellify->tui);
                } else if (key == shellify->config->keys.super) {
                    handle_form_clipboard(shellify->tui->input_form);
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP_LINK:
            if (shellify->input_state == INPUT_STATE_WRITING) {
                if (handle_input_form(key, shellify->tui->input_form,
                                      shellify->config)) {
                    TUI_InputForm* form = shellify->tui->input_form;
                    DLTask* t = dlq_task(shellify->stg->dlq, form->values[0],
                                         form->values[1], form->values[2],
                                         form->values[3]);
                    if (!t) {
                        clear_input_form(shellify->tui);
                        return;
                    }

                    dlq_push(shellify->stg->dlq, t);
                    free(t);
                    clear_input_form(shellify->tui);
                    shellify->state = SHELLIFY_STATE_PLAYER;
                }
            } else {
                if (key == shellify->config->keys.select) {
                    shellify->input_state = INPUT_STATE_WRITING;
                } else if (key == KEY_ARROW_LEFT) {
                    shellify->state = SHELLIFY_STATE_ADD_SONG;
                    clear_input_form(shellify->tui);
                } else if (key == shellify->config->keys.super) {
                    handle_form_clipboard(shellify->tui->input_form);
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG_YTDLP_SEARCH:
            if (shellify->input_state == INPUT_STATE_WRITING) {
                handle_search_form_typing(key, shellify->tui->search_form);
            } else {
                if (key == shellify->config->keys.super) {
                    shellify->input_state = INPUT_STATE_WRITING;
                } else if (key == KEY_ARROW_LEFT) {
                    shellify->state = SHELLIFY_STATE_PLAYER;
                    clear_search_form(shellify->tui);
                }

                int idx = handle_choice_form(
                    key, shellify->tui->search_form->form, shellify->config);
                if (idx >= 0) {
                    add_song_search(shellify->tui, shellify->stg, idx);
                    shellify->state = SHELLIFY_STATE_PLAYER;
                    clear_search_form(shellify->tui);
                }
            }
            break;
        case SHELLIFY_STATE_ADD_SONG_IMPORT:
            if (shellify->input_state == INPUT_STATE_WRITING) {
                handle_search_form_typing(key, shellify->tui->search_form);
            } else {
                if (key == KEY_ARROW_LEFT) {
                    shellify->state = SHELLIFY_STATE_PLAYER;
                    clear_search_form(shellify->tui);
                    clear_import_data(&shellify->stg->import_data);
                    break;
                }

                int idx = handle_choice_form(
                    key, shellify->tui->search_form->form, shellify->config);
                if (idx >= 0) {
                    add_song_import(shellify->tui, shellify->stg, idx,
                                    shellify->stg->import_data->idx);
                    if (shellify->stg->import_data->idx >
                        shellify->stg->import_data->size) {
                        shellify->state = SHELLIFY_STATE_PLAYER;
                        clear_search_form(shellify->tui);
                        clear_import_data(&shellify->stg->import_data);
                        break;
                    }

                    size_t* data_idx = &shellify->stg->import_data->idx;
                    (*data_idx)++;

                    char* query = build_query_import(
                        &shellify->stg->import_data->entries[*data_idx]);
                    search_run(shellify->stg->sr_core, query);
                    free(query);
                }
            }
            break;
        case SHELLIFY_STATE_ADD_PLAYLIST:
            if (shellify->input_state == INPUT_STATE_WRITING) {
                if (handle_input_form(key, shellify->tui->input_form,
                                      shellify->config)) {
                    add_plist(shellify->tui, shellify->stg);
                    clear_input_form(shellify->tui);
                    shellify->state = SHELLIFY_STATE_PLAYER;
                }
            } else {
                if (key == shellify->config->keys.select) {
                    shellify->input_state = INPUT_STATE_WRITING;
                } else if (key == KEY_ARROW_LEFT) {
                    shellify->state = SHELLIFY_STATE_PLAYER;
                    clear_input_form(shellify->tui);
                } else if (key == shellify->config->keys.super) {
                    handle_form_clipboard(shellify->tui->input_form);
                }
            }
            break;
        case SHELLIFY_STATE_DASHBOARD:
            if (key == KEY_ARROW_LEFT) {
                shellify->state = SHELLIFY_STATE_PLAYER;
            }
            break;
        case SHELLIFY_STATE_UPDATE_SONG:
            if (shellify->input_state == INPUT_STATE_WRITING) {
                if (handle_input_form(key, shellify->tui->input_form,
                                      shellify->config)) {
                    Playlist* plist =
                        shellify->stg->lib
                            ->playlists[shellify->tui->idx_plists];
                    Song* song = plist->songs[shellify->tui->idx_songs];
                    TUI_InputForm* form = shellify->tui->input_form;
                    lib_update_sng(shellify->stg->lib, song, form->values[0],
                                   form->values[1], form->values[2]);
                    stg_update_sng(shellify->stg, song, form->values[0],
                                   form->values[1], form->values[2]);
                    clear_input_form(shellify->tui);
                    shellify->input_state = INPUT_STATE_NONE;
                    shellify->state = SHELLIFY_STATE_PLAYER;
                }
            } else {
                if (key == shellify->config->keys.select) {
                    shellify->input_state = INPUT_STATE_WRITING;
                } else if (key == KEY_ARROW_LEFT) {
                    shellify->state = SHELLIFY_STATE_PLAYER;
                    clear_input_form(shellify->tui);
                } else if (key == shellify->config->keys.super) {
                    handle_form_clipboard(shellify->tui->input_form);
                }
            }
            break;
        default:
            shellify->state = SHELLIFY_STATE_PLAYER;
            break;
    }
}

void shellify_sleep() { usleep(shellify->config->general.usleep); }

void shellify_stop() {
    slog(WARNING, "shellify is forced to close.");
    shellify->is_running = 0;
}

#include "tui.h"

#include <stdlib.h>

#include "buffer.h"
#include "dl_queue.h"
#include "rect.h"

int tui_init(TUI** tui, size_t* window_cols, size_t* window_rows) {
    if (!tui || !window_cols || !window_rows) {
        errlog(ERR_NULL_OBJECT, "tui:init_tui:args");
        return 0;
    }

    TUI* temp = malloc(sizeof(TUI));
    if (!temp) {
        errlog(ERR_MALLOC_NULL, "tui:init_tui:temp");
        return 0;
    }

    size_t size = *window_cols + 1;
    // creating separator
    temp->separator = malloc(size * sizeof(char));
    if (!temp->separator) {
        errlog(ERR_MALLOC_NULL, "tui:init_tui:sep");
        free(temp);
        return 0;
    }

    memset(temp->separator, '_', size * sizeof(char));
    temp->separator[*window_cols] = '\0';

    // creating song name line
#define MAX_LEN_SONG 96
    temp->song_name = malloc(MAX_LEN_SONG * sizeof(char));
    if (!temp->song_name) {
        errlog(ERR_MALLOC_NULL, "tui:init_tui:song_name");
        free(temp->separator);
        free(temp);
        return 0;
    }

    temp->song_name[0] = '\0';

    temp->idx_plists = 0, temp->idx_songs = 0;
    temp->scroll_plists = 0, temp->scroll_songs = 0;
    temp->offset = 0;
    temp->changed = 0;
    temp->input_form = NULL;
    temp->choice_form = NULL;
    tui_update(temp, window_cols, window_rows);

    *tui = temp;
    slog(INFO, "tui has been init");
    return 1;
}

void tui_update(TUI* tui, size_t* window_cols, size_t* window_rows) {
    if (!tui || !window_cols || !window_rows) return;

#define TUI_SPLIT_SCREEN 0.2
    tui->header_top_border = 2;
    tui->header_bottom_border = *window_rows - 3;
    tui->playlist_wall = (size_t)*window_cols * TUI_SPLIT_SCREEN;
    tui->x_playlists = 2;
    tui->y_playlists = tui->header_top_border + 2;
    tui->x_songs = tui->playlist_wall + 3;
    tui->y_songs = tui->header_top_border + 2;
    slog(INFO, "tui parameters has been updated");
}

void tui_sync(TUI* tui, Storage* stg) {
    if (!tui || !stg) return;

    Playlist* playlist = stg->lib->playlists[tui->idx_plists];
    size_t    max_songs = tui->header_bottom_border - tui->y_songs - 1;

    if (tui->idx_songs < tui->scroll_songs) {
        tui->scroll_songs = tui->idx_songs;
    }

    if (tui->idx_songs >= tui->scroll_songs + max_songs) {
        tui->scroll_songs = tui->idx_songs - max_songs + 1;
    }
}

void tui_clear(TUI* tui) {
    if (!tui) return;

    if (tui->separator) free(tui->separator);
    if (tui->song_name) free(tui->song_name);

    if (tui->input_form) {
        clear_input_form(tui);
    }
    if (tui->choice_form) {
        clear_choice_form(tui);
    }

    slog(INFO, "tui has been cleaned");
}

// tui elements and inteface

// bosses

void make_welcome(TUI* tui, Buffer* buffer, Config* config) {
    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);
    draw_rect(buffer, rect);

#define BUFFER_BASE_SIZE 256 * sizeof(char)
    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "tui:make_welcome:buf");
        return;
    }

    snprintf(buf, BUFFER_BASE_SIZE, "%s %s", config->general.name,
             config->general.version);

    size_t text_x = rect.vec.x + (rect.w - strlen(buf)) / 2;
    size_t desc_x = rect.vec.x + (rect.w - strlen(config->general.desc)) / 2;
    size_t git_x = rect.vec.x + (rect.w - GIT_MSG_LEN) / 2;

    buffer_append_line_styled(buffer, (Vec){text_x, rect.vec.y + 3}, buf,
                              COLOR_CYAN, COLOR_DEFAULT, STYLE_BOLD);
    buffer_append_line_styled(buffer, (Vec){desc_x, rect.vec.y + 5},
                              config->general.desc, COLOR_DEFAULT,
                              COLOR_DEFAULT, STYLE_UNDERLINE);
    buffer_append_line_styled(buffer, (Vec){git_x, rect.vec.y + 9}, GIT_MSG,
                              COLOR_RED, COLOR_DEFAULT, STYLE_BOLD);

    snprintf(buf, BUFFER_BASE_SIZE,
             "Press SELECT-BUTTON (%c) to start shellify", config->keys.select);
    size_t msg_x = rect.vec.x + (rect.w - strlen(buf)) / 2;
    buffer_append_line(buffer, (Vec){msg_x, rect.vec.y + 16}, buf);
    free(buf);
}

void make_header(TUI* tui, Storage* stg, Buffer* buffer, Audio* audio,
                 Config* config, const char* mode) {
    if (!tui || !buffer || !audio || !config) return;

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "tui:make_header:buf");
        return;
    }

    // top side
    snprintf(buf, BUFFER_BASE_SIZE, "%s %s", config->general.name,
             config->general.version);
    buffer_append_line_styled(buffer, (Vec){0, 0}, buf, COLOR_CYAN,
                              COLOR_DEFAULT, STYLE_BOLD);

    snprintf(buf, BUFFER_BASE_SIZE,
             "help -> super: %c; select: %c; add: %c; remove: %c; song: %c; "
             "playlist: %c; increase vol: %c; decrease vol: %c; shuffle "
             "on/off: %c; dashboard: %c",
             config->keys.super, config->keys.select, config->keys.add,
             config->keys.remove, config->keys.song, config->keys.playlist,
             config->keys.inc, config->keys.dec, config->keys.shuffle,
             config->keys.dashboard);
    tui->offset = buffer_append_line_offset(buffer, (Vec){0, 1}, buf);
    if (!tui->changed) {
        tui->header_top_border += tui->offset;
        tui->changed = 1;
    }

    snprintf(buf, BUFFER_BASE_SIZE, "Volume -> %.2f", config->player.volume);
    buffer_append_line_styled(buffer,
                              (Vec){buffer->window_cols - 30, 1 + tui->offset},
                              buf, COLOR_DEFAULT, COLOR_DEFAULT, STYLE_BOLD);

    snprintf(buf, BUFFER_BASE_SIZE, "Shuffle -> %zu", config->player.shuffle);
    buffer_append_line_styled(buffer,
                              (Vec){buffer->window_cols - 13, 1 + tui->offset},
                              buf, COLOR_DEFAULT, COLOR_DEFAULT, STYLE_BOLD);

    buffer_append_line(buffer, (Vec){0, tui->header_top_border},
                       tui->separator);

    // bottom side
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border},
                       tui->separator);

    updating_cur_song(tui, stg, audio);

    snprintf(buf, BUFFER_BASE_SIZE, "%s %s", PREFIX_PLAYING, tui->song_name);
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border + 1}, buf);

    snprintf(buf, BUFFER_BASE_SIZE, "Mode = [ %s ]", mode);
    buffer_append_line(
        buffer, (Vec){buffer->window_cols - 20, tui->header_bottom_border + 1},
        buf);

    free(buf);
}

void make_player(TUI* tui, Storage* stg, Buffer* buffer, Audio* audio,
                 Config* config, int focus) {
    buffer_set_ver_range_char(
        buffer, (Vec){tui->header_top_border, tui->header_bottom_border + 1},
        (Vec){tui->playlist_wall, tui->header_top_border - 1}, '|');

    view_plists(tui, stg, buffer);
    view_songs(tui, stg, buffer, audio);

    Rect rect = (Rect){(Vec){0, 0}, 0, 0};
    switch (focus) {
        case 0:
            rect = (Rect){
                (Vec){1, tui->header_top_border + 1}, tui->playlist_wall - 1,
                tui->header_bottom_border - tui->header_top_border - 1};

            draw_rect(buffer, rect);
            break;
        case 1:
            rect = (Rect){
                (Vec){tui->playlist_wall + 2, tui->header_top_border + 1},
                buffer->window_cols - tui->playlist_wall - 2,
                tui->header_bottom_border - tui->header_top_border - 1};
            draw_rect(buffer, rect);
            break;
        default:
            break;
    }
}

// views

void view_plists(TUI* tui, Storage* stg, Buffer* buffer) {
    size_t x = tui->x_playlists;
    size_t y = tui->y_playlists + tui->offset;

    buffer_append_line(buffer, (Vec){x, y}, "[ PLAYLISTS ]");

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) return;

    for (size_t i = 0; i < stg->lib->playlist_count; i++) {
        if (y + i + 3 >= tui->header_bottom_border) break;

        if (i == tui->idx_plists) {
            snprintf(buf, BUFFER_BASE_SIZE, "> %s",
                     stg->lib->playlists[i]->name);
        } else {
            snprintf(buf, BUFFER_BASE_SIZE, "  %s",
                     stg->lib->playlists[i]->name);
        }

        buffer_append_line(buffer, (Vec){x, y + i + 3}, buf);
    }
    free(buf);
}

void view_songs(TUI* tui, Storage* stg, Buffer* buffer, Audio* audio) {
    size_t x = tui->x_songs;
    size_t y = tui->y_songs + tui->offset;

    buffer_append_line(buffer, (Vec){x, y}, "[ SONGS ]");

    if (stg->lib->playlist_count == 0) {
        buffer_append_line(buffer, (Vec){x, y + 3}, "(No playlists!!!)");
        return;
    }

    Playlist* playlist = stg->lib->playlists[tui->idx_plists];

    if (playlist->song_count == 0) {
        buffer_append_line(buffer, (Vec){x, y + 3}, "(No songs!!!!)");
        return;
    }

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) return;

    size_t max = tui->header_bottom_border - y - 1;
    for (size_t i = tui->scroll_songs; i < playlist->song_count; i++) {
        if (i - tui->scroll_songs >= max) break;

        const char* format = NULL;
        Song*       song = playlist->songs[i];
        if (tui->idx_songs != audio->sng_idx && i == audio->sng_idx &&
            tui->idx_plists == audio->plist_idx) {
            format = "& [%zu] %s - %s  -  %s";
        } else {
            if (i == tui->idx_songs) {
                format = "> [%zu] %s - %s  -  %s";
            } else {
                format = "  [%zu] %s - %s  -  %s";
            }
        }

        char       time[20];
        struct tm* t = localtime(&song->time);
        strftime(time, sizeof(time), "%b %d %H:%M", t);
        snprintf(buf, BUFFER_BASE_SIZE, format, i + 1, song->artist,
                 song->title, song->album, time);
        buffer_append_line(buffer, (Vec){x, y + (i - tui->scroll_songs) + 3},
                           buf);
    }
    free(buf);
}

// ADD song

void make_add_sn(TUI* tui, Buffer* buffer, Config* config) {
    size_t size = 3;
    if (!tui->choice_form) {
        const char* options[3] = {"load from local files",
                                  "from youtube (yt-dlp, forward link)",
                                  "from youtube (yt-dlp, search)"};

        set_choice_form(tui, options, size);
    }

    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) return;
    snprintf(buf, BUFFER_BASE_SIZE,
             "Press SELECT (%c) to choose, Press LEFT to leave",
             config->keys.select);
    make_choice_form(tui, buffer, rect, buf);
    free(buf);
}

void make_add_local_sn(TUI* tui, Buffer* buffer, Config* config) {
    size_t size = 4;
    if (!tui->input_form) {
        const char* options[4] = {"Path: ", "Title: ", "Artist : ", "Album : "};

        set_input_form(tui, options, size);
    }

    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);

    make_input_form(tui, buffer, rect,
                    "UP/DOWN: moving, RIGHT to choose, LEFT to leave");
}

void make_add_ytdlp_sn_link(TUI* tui, Buffer* buffer, Config* config) {
    size_t size = 4;
    if (!tui->input_form) {
        const char* options[4] = {"URL: ", "Title: ", "Artist: ", "Album: "};

        set_input_form(tui, options, size);
    }

    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);

    make_input_form(tui, buffer, rect,
                    "UP/DOWN: moving, RIGHT to choose, LEFT to leave");
}

void make_add_ytdlp_sn_search(TUI* tui, Buffer* buffer, Config* config) {}

// ADD playlist

void make_add_plist(TUI* tui, Buffer* buffer, Config* config) {
    size_t size = 1;
    if (!tui->input_form) {
        const char* options[1] = {"Name: "};

        set_input_form(tui, options, size);
    }

    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);

    make_input_form(tui, buffer, rect, "RIGHT to apply, LEFT to leave");
}

// MAKE dashboard
void make_dashboard(TUI* tui, Storage* stg, Buffer* buffer, Config* config) {
    if (!tui || !stg || !buffer) return;

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) return;

    size_t y = tui->header_top_border + 1;
    size_t x = 2;
    // >>
    const char* sign = " [ DASHBOARD ]";
    size_t      dx = (buffer->window_cols / 2) - (strlen(sign) / 2) - 1;
    buffer_append_line_styled(buffer, (Vec){dx, y++}, sign, COLOR_BLUE,
                              COLOR_DEFAULT, STYLE_BOLD);
    // >>
    Rect rect = (Rect){0, y++, buffer->window_cols,
                       buffer->window_rows - tui->header_top_border - 5};
    draw_rect(buffer, rect);
    // >>
    buffer_append_line_styled(buffer, (Vec){x, y++}, config->general.name,
                              COLOR_CYAN, COLOR_DEFAULT, STYLE_BOLD);

    snprintf(buf, BUFFER_BASE_SIZE, "current version: %s",
             config->general.version);
    buffer_append_line_styled(buffer, (Vec){x, y++}, buf, COLOR_DEFAULT,
                              COLOR_DEFAULT, STYLE_BOLD);
    // >>
    buffer_append_line(buffer, (Vec){0, y++}, tui->separator);
    // >>
    size_t firewall = (size_t)(buffer->window_cols * 0.3);
    buffer_set_ver_range_char(
        buffer, (Vec){0, buffer->window_rows - tui->header_top_border - 10},
        (Vec){firewall, y}, '|');
    // >>
    DLIterator* it = dli_init(stg->dlq);
    for (size_t i = 0; i < stg->dlq->size; i++) {
        snprintf(buf, BUFFER_BASE_SIZE, ">%zu -> %s", i, dli_next(it)->title);
        buffer_append_line(buffer, (Vec){x, y++}, buf);
    }
    dli_close(it);
}

// >>> input form handler

void create_input_form(TUI* tui, size_t cap) {
    if (tui->input_form) {
        clear_input_form(tui);
        tui->input_form = NULL;
    }

    TUI_InputForm* form = malloc(sizeof(TUI_InputForm));
    if (!form) {
        errlog(ERR_MALLOC_NULL, "tui:create_input_form:form");
        return;
    }

    form->cap = cap;
    form->size = 0;
    form->selected_option = 0;
    form->str_len = BUFFER_BASE_SIZE;

    form->options = malloc(cap * sizeof(char*));
    form->values = malloc(cap * sizeof(char*));

    for (size_t i = 0; i < cap; i++) {
        form->options[i] = malloc(form->str_len * sizeof(char));
        form->options[i][0] = '\0';

        form->values[i] = malloc(form->str_len * sizeof(char));
        form->values[i][0] = '\0';
    }

    tui->input_form = form;
}

void set_input_form(TUI* tui, const char* options[], size_t cap) {
    if (tui->input_form) {
        clear_input_form(tui);
        tui->input_form = NULL;
    }

    create_input_form(tui, cap);
    for (size_t i = 0; i < cap; i++) {
        put_inform(tui->input_form, i, options[i]);
    }
}

void clear_input_form(TUI* tui) {
    if (!tui->input_form) return;

    for (size_t i = 0; i < tui->input_form->cap; i++) {
        free(tui->input_form->options[i]);
        free(tui->input_form->values[i]);
    }

    free(tui->input_form->options);
    free(tui->input_form->values);

    free(tui->input_form);
    tui->input_form = NULL;
}

void put_inform(TUI_InputForm* form, size_t idx, const char* msg) {
    if (!form || !msg) return;
    if (idx >= form->cap) return;

    size_t max_len = form->str_len - 1;
    strncpy(form->options[idx], msg, max_len);
    form->options[idx][max_len] = '\0';

    form->size = idx + 1;
}

void make_input_form(TUI* tui, Buffer* buffer, Rect rect, const char* msg) {
    if (!tui->input_form) return;

    draw_rect(buffer, rect);

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) return;

    for (size_t i = 0; i < tui->input_form->size; i++) {
        if (i == tui->input_form->selected_option) {
            snprintf(buf, BUFFER_BASE_SIZE, "> %s %s_",
                     tui->input_form->options[i], tui->input_form->values[i]);
        } else {
            snprintf(buf, BUFFER_BASE_SIZE, "  %s %s",
                     tui->input_form->options[i], tui->input_form->values[i]);
        }

        buffer_append_line(
            buffer, (Vec){rect.vec.x + 4, rect.vec.y + 3 + (i * 2)}, buf);
    }

    if (msg) {
        buffer_append_line(
            buffer,
            (Vec){rect.vec.x + (rect.w - strlen(msg)) / 2, rect.vec.y + 15},
            msg);
    }

    free(buf);
}

// >>> choice form handler

void create_choice_form(TUI* tui, size_t cap) {
    if (tui->choice_form) {
        clear_choice_form(tui);
        tui->choice_form = NULL;
    }

    TUI_ChoiceForm* form = malloc(sizeof(TUI_ChoiceForm));
    if (!form) {
        errlog(ERR_MALLOC_NULL, "tui:create_choice_form:form");
        return;
    }

    form->cap = cap;
    form->size = 0;
    form->selected_option = 0;
    form->str_len = BUFFER_BASE_SIZE;

    form->options = malloc(cap * sizeof(char*));

    for (size_t i = 0; i < cap; i++) {
        form->options[i] = malloc(form->str_len * sizeof(char));
        form->options[i][0] = '\0';
    }

    tui->choice_form = form;
}

void set_choice_form(TUI* tui, const char* options[], size_t cap) {
    if (tui->choice_form) {
        clear_choice_form(tui);
        tui->choice_form = NULL;
    }

    create_choice_form(tui, cap);
    for (size_t i = 0; i < cap; i++) {
        put_chform(tui->choice_form, i, options[i]);
    }
}

void clear_choice_form(TUI* tui) {
    if (!tui->choice_form) return;

    for (size_t i = 0; i < tui->choice_form->cap; i++) {
        free(tui->choice_form->options[i]);
    }

    free(tui->choice_form->options);
    free(tui->choice_form);

    tui->choice_form = NULL;
}

void put_chform(TUI_ChoiceForm* form, size_t idx, const char* msg) {
    if (!form || !msg) return;
    if (idx >= form->cap) return;

    size_t max_len = form->str_len - 1;
    strncpy(form->options[idx], msg, max_len);
    form->options[idx][max_len] = '\0';

    form->size = idx + 1;
}

void make_choice_form(TUI* tui, Buffer* buffer, Rect rect, const char* msg) {
    if (!tui->choice_form) return;

    draw_rect(buffer, rect);

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) return;

    for (size_t i = 0; i < tui->choice_form->size; i++) {
        if (i == tui->choice_form->selected_option) {
            snprintf(buf, BUFFER_BASE_SIZE, " > %s ",
                     tui->choice_form->options[i]);
        } else {
            snprintf(buf, BUFFER_BASE_SIZE, "   %s ",
                     tui->choice_form->options[i]);
        }

        buffer_append_line(buffer, (Vec){rect.vec.x + 4, rect.vec.y + 3 + i},
                           buf);
    }

    if (msg) {
        buffer_append_line(
            buffer,
            (Vec){rect.vec.x + (rect.w - strlen(msg)) / 2, rect.vec.y + 16},
            msg);
    }

    free(buf);
}

// >>> utils

void updating_cur_song(TUI* tui, Storage* stg, Audio* audio) {
    if (!tui || !stg || !audio || !audio->is_init || !audio->is_sound) return;
    if (audio->plist_idx >= stg->lib->playlist_count ||
        audio->sng_idx >= stg->lib->song_count)
        return;

#define MSG_PAUSED "[ PAUSED ]"
    const char* n =
        stg->lib->playlists[audio->plist_idx]->songs[audio->sng_idx]->title;
    if (audio->is_stopped) {
        snprintf(tui->song_name, MAX_LEN_SONG, "%s %s", n, MSG_PAUSED);
    } else {
        snprintf(tui->song_name, MAX_LEN_SONG, "%s", n);
    }
}

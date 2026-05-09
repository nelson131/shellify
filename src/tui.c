#include "tui.h"

#include <stdlib.h>

#include "rect.h"

int tui_init(TUI** tui, size_t* window_cols, size_t* window_rows) {
    if (!tui || !window_cols || !window_rows) {
        raise_error(ERR_NULL_OBJECT, "tui:init_tui:args");
        return 0;
    }

    TUI* temp = malloc(sizeof(TUI));
    if (!temp) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:temp");
        return 0;
    }

    size_t size = *window_cols + 1;
    // creating separator
    temp->separator = malloc(size * sizeof(char));
    if (!temp->separator) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:sep");
        free(temp);
        return 0;
    }

    memset(temp->separator, '_', size * sizeof(char));
    temp->separator[*window_cols] = '\0';

    // creating song name line
#define MAX_LEN_SONG 96
    temp->song_name = malloc(MAX_LEN_SONG * sizeof(char));
    if (!temp->song_name) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:song_name");
        free(temp->separator);
        free(temp);
        return 0;
    }

    temp->song_name[0] = '\0';

    temp->input_form = NULL;
    temp->choice_form = NULL;
    tui_update(temp, window_cols, window_rows);

    *tui = temp;
    return 1;
}

void tui_update(TUI* tui, size_t* window_cols, size_t* window_rows) {
    if (!tui || !window_cols || !window_rows) return;

#define TUI_SPLIT_SCREEN 0.2
    tui->header_top_border = 2;
    tui->header_bottom_border = *window_rows - 3;
    tui->playlist_wall = (size_t)*window_cols * TUI_SPLIT_SCREEN;
    tui->x_playlists = 1;
    tui->y_playlists = tui->header_top_border + 1;
    tui->x_songs = tui->playlist_wall + 2;
    tui->y_songs = tui->header_top_border + 1;
}

void tui_clear(TUI* tui) {
    if (!tui) return;

    if (tui->separator) free(tui->separator);
    if (tui->song_name) free(tui->song_name);

    if (tui->input_form) {
        clear_input_form(tui->input_form);
        tui->input_form = NULL;
    }
    if (tui->choice_form) {
        clear_choice_form(tui->choice_form);
        tui->choice_form = NULL;
    }
}

// tui elements and inteface

void make_welcome(TUI* tui, Buffer* buffer, Config* config) {
    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);
    draw_rect(buffer, rect);

#define BUFFER_BASE_SIZE 256 * sizeof(char)
    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:make_welcome:buf");
        return;
    }

    snprintf(buf, BUFFER_BASE_SIZE, "%s %s", config->general.name,
             config->general.version);

    size_t text_x = rect.vec.x + (rect.w - strlen(buf)) / 2;
    size_t desc_x = rect.vec.x + (rect.w - strlen(config->general.desc)) / 2;

    buffer_append_line(buffer, (Vec){text_x, rect.vec.y + 3}, buf);
    buffer_append_line(buffer, (Vec){desc_x, rect.vec.y + 5},
                       config->general.desc);

    snprintf(buf, BUFFER_BASE_SIZE,
             "Press SELECT-BUTTON (%c) to start shellify", config->keys.select);
    size_t msg_x = rect.vec.x + (rect.w - strlen(buf)) / 2;
    buffer_append_line(buffer, (Vec){msg_x, rect.vec.y + 16}, buf);
    free(buf);
}

void make_header(TUI* tui, Buffer* buffer, Config* config, const char* mode) {
    if (!tui || !buffer || !config) return;

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:make_header:buf");
        return;
    }

    // top side
    snprintf(buf, BUFFER_BASE_SIZE, "%s %s", config->general.name,
             config->general.version);
    buffer_append_line(buffer, (Vec){0, 0}, buf);

    snprintf(buf, BUFFER_BASE_SIZE,
             "help -> super: %c; select: %c; add: %c; remove: %c; song: %c; "
             "playlist: %c;",
             config->keys.super, config->keys.select, config->keys.add,
             config->keys.remove, config->keys.song, config->keys.playlist);
    buffer_append_line(buffer, (Vec){0, 1}, buf);

    buffer_append_line(buffer, (Vec){0, tui->header_top_border},
                       tui->separator);

    // bottom side
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border},
                       tui->separator);

    snprintf(buf, BUFFER_BASE_SIZE, "%s %s", PREFIX_PLAYING, tui->song_name);
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border + 1}, buf);
    size_t len = strlen(buf);

    snprintf(buf, BUFFER_BASE_SIZE, "Mode = [ %s ]", mode);
    buffer_append_line(buffer, (Vec){len + 10, tui->header_bottom_border + 1},
                       buf);
    free(buf);
}

void make_player(TUI* tui, Library* library, Buffer* buffer, Config* config) {
    buffer_set_ver_range_char(
        buffer, (Vec){tui->header_top_border, tui->header_bottom_border + 1},
        (Vec){tui->playlist_wall, tui->header_top_border - 1}, '|');
}

// ADD song

void make_add_sn(TUI* tui, Buffer* buffer, Config* config) {
    size_t size = 2;
    if (!tui->choice_form) {
        const char* options[2] = {"load from local files",
                                  "download from youtube using yt-dlp"};

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
    size_t size = 5;
    if (!tui->input_form) {
        const char* options[5] = {
            "Playlist-id: ", "Path: ", "Title: ", "Artist : ", " Album : "};

        set_choice_form(tui, options, size);
    }

    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    rect_center(&rect, buffer->window_cols, buffer->window_rows);

    make_choice_form(tui, buffer, rect,
                     "UP/DOWN: moving, SELECT to choose, LEFT to leave");
}

// >>> input form handler

void create_input_form(TUI* tui, size_t cap) {
    if (tui->input_form) {
        clear_input_form(tui->input_form);
        tui->input_form = NULL;
    }

    TUI_InputForm* form = malloc(sizeof(TUI_InputForm));
    if (!form) {
        raise_error(ERR_MALLOC_NULL, "tui:create_input_form:form");
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
        clear_input_form(tui->input_form);
        tui->input_form = NULL;
    }

    create_input_form(tui, cap);
    for (size_t i = 0; i < cap; i++) {
        put_inform(tui->input_form, i, options[i]);
    }
}

void clear_input_form(TUI_InputForm* form) {
    if (!form) return;

    for (size_t i = 0; i < form->cap; i++) {
        free(form->options[i]);
        free(form->values[i]);
    }

    free(form->options);
    free(form->values);

    free(form);
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
        clear_choice_form(tui->choice_form);
        tui->choice_form = NULL;
    }

    TUI_ChoiceForm* form = malloc(sizeof(TUI_ChoiceForm));
    if (!form) {
        raise_error(ERR_MALLOC_NULL, "tui:create_choice_form:form");
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
        clear_choice_form(tui->choice_form);
        tui->choice_form = NULL;
    }

    create_choice_form(tui, cap);
    for (size_t i = 0; i < cap; i++) {
        put_chform(tui->choice_form, i, options[i]);
    }
}

void clear_choice_form(TUI_ChoiceForm* form) {
    if (!form) return;

    for (size_t i = 0; i < form->cap; i++) {
        free(form->options[i]);
    }

    free(form->options);
    free(form);
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

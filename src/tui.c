#include "tui.h"

#include <stdio.h>

#include "buffer.h"

// Initialization of main tui struct:
// handling separator line, now playing line and
// current playing song in the heap
int tui_init(TUI** tui, size_t* window_cols, size_t* window_rows) {
    TUI* tui_temp = malloc(sizeof(TUI));
    if (!tui_temp) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:tui");
        return 0;
    }

    char* temp = malloc((*window_cols + 1) * sizeof(char));
    if (!temp) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:separator");
        return 0;
    }

    for (size_t i = 0; i < *window_cols; i++) {
        temp[i] = '_';
    }

    temp[*window_cols] = '\0';
    tui_temp->separator = temp;

#define PREFIX_SIZE strlen(PREFIX_PLAYING)
    size_t str_size = ((*window_cols - PREFIX_SIZE) + 1) * sizeof(char);
    char*  line_status = malloc(str_size);
    if (!line_status) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:now_playing");
        return 0;
    }

    tui_temp->line_status = line_status;

#define MAX_SONG_LEN 64
    char* line_song = calloc(' ', MAX_SONG_LEN * sizeof(char));
    if (!line_song) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:song_name");
        return 0;
    }

    tui_temp->song_name = line_song;

    snprintf(tui_temp->line_status, str_size, " %s%s\n", PREFIX_PLAYING,
             tui_temp->song_name);

    tui_update(tui_temp, window_cols, window_rows);
    tui_temp->input_form = NULL;

    *tui = tui_temp;
    return 1;
}

// clearing the heap lines from tui struct
void tui_clear(TUI* tui) {
    if (tui->separator) {
        free(tui->separator);
    }

    if (tui->line_status) {
        free(tui->line_status);
    }

    if (tui->song_name) {
        free(tui->song_name);
    }

    if (tui->input_form) {
        clear_input_form(tui->input_form);
    }
}

// updating the tui variables for rendering
// should be called when screen size is updated
void tui_update(TUI* tui, size_t* window_cols, size_t* window_rows) {
#define TUI_SPLIT_SCREEN 0.2
    tui->header_top_border = 2;
    tui->header_bottom_border = *window_rows - 3;
    tui->playlist_wall = (size_t)*window_cols * TUI_SPLIT_SCREEN;
    tui->x_playlists = 1;
    tui->y_playlists = tui->header_top_border + 1;
    tui->x_songs = tui->playlist_wall + 2;
    tui->y_songs = tui->header_top_border + 1;
}

int create_header(TUI* tui, Buffer* buffer, Config* config) {
#define BUFFER_BASE_SIZE 128 * sizeof(char)
    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:create_header:headline");
        return 0;
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
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border + 1},
                       tui->line_status);

    free(buf);
    return 1;
}

// creating the welcome menu
int create_welcome(TUI* tui, Buffer* buffer, Config* config) {
    Rect rect = {(Vec){0, 0}, 60, 20};
    if (buffer->window_cols > rect.w) {
        rect.vec.x = (buffer->window_cols - rect.w) / 2;
    }
    if (buffer->window_rows > rect.h) {
        rect.vec.y = (buffer->window_rows - rect.h) / 2;
    }

    draw_rect(buffer, rect);

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:create_welcome:buf");
        return 0;
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
    return 1;
}

// creating the main tui screen of shellify
// handling the rendering of playlists, songs, selecting
// playing song
int create_player(TUI* tui, Library* library, Buffer* buffer, Config* config) {
    buffer_set_ver_range_char(
        buffer, (Vec){tui->header_top_border, tui->header_bottom_border + 1},
        (Vec){tui->playlist_wall, tui->header_top_border - 1}, '|');

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:create_player:buf");
        return 0;
    }

    // rendering playlists
    for (size_t i = 0; i < library->playlist_count; i++) {
        snprintf(buf, BUFFER_BASE_SIZE, "%zu. %s", i + 1,
                 library->playlists[i]->name);
        buffer_append_line(buffer,
                           (Vec){tui->x_playlists, tui->y_playlists + i}, buf);
    }

    free(buf);
    return 1;
}

// creating the add new song menu with available adding
// sources, like local files and yt-dlp
int create_add_menu(TUI* tui, Buffer* buffer, Config* config) {
    Rect rect = (Rect){(Vec){0, 0}, 60, 20};
    if (buffer->window_cols > rect.w) {
        rect.vec.x = (buffer->window_cols - rect.w) / 2;
    }
    if (buffer->window_rows > rect.h) {
        rect.vec.y = (buffer->window_rows - rect.h) / 2;
    }

    draw_rect(buffer, rect);

    const char* title = "[ ADD NEW SONG ]";
    size_t      title_x = rect.vec.x + (rect.w - strlen(title)) / 2;

    buffer_append_line(buffer, (Vec){title_x, rect.vec.y + 1}, title);

    size_t      options_size = 2;
    const char* options[2] = {"load from local files",
                              "download from youtube using yt-dlp"};

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:create_add_menu:buf");
        return 0;
    }

    for (size_t i = 0; i < options_size; i++) {
        if (i == tui->selected_index) {
            snprintf(buf, BUFFER_BASE_SIZE, " > %s ", options[i]);
        } else {
            snprintf(buf, BUFFER_BASE_SIZE, "   %s ", options[i]);
        }

        buffer_append_line(buffer, (Vec){rect.vec.x + 4, rect.vec.y + 3 + i},
                           buf);
    }

    snprintf(buf, BUFFER_BASE_SIZE,
             "Press SELECT (%c) to choose, Press LEFT to leave",
             config->keys.select);
    size_t msg_x = rect.vec.x + (rect.w - strlen(buf)) / 2;

    buffer_append_line(buffer, (Vec){msg_x, rect.vec.y + 16}, buf);

    free(buf);
    return 1;
}

// creating the input form for user
// handling the options and values arrays of the same sizes
TUI_InputForm* create_input_form(size_t cap) {
    TUI_InputForm* form = malloc(sizeof(TUI_InputForm));
    if (!form) {
        raise_error(ERR_MALLOC_NULL, "tui:create_input_form:form");
        return NULL;
    }

    form->cap = cap;
    form->size = 0;
    form->selected_option = 0;
    form->str_len = BUFFER_BASE_SIZE;

    form->options = malloc(cap * sizeof(char*));
    form->values = malloc(cap * sizeof(char*));
    if (!form->options || !form->values) {
        raise_error(ERR_MALLOC_NULL, "tui:create_input_form:options/values");

        if (form->options) {
            free(form->options);
        }
        if (form->values) {
            free(form->values);
        }
        free(form);
        return NULL;
    }

    for (size_t i = 0; i < cap; i++) {
        form->options[i] = malloc(form->str_len * sizeof(char));
        form->options[i][0] = '\0';

        form->values[i] = malloc(form->str_len * sizeof(char));
        form->values[i][0] = '\0';
    }

    return form;
}

// clearing the arrays of input form
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

void put_in_form(TUI_InputForm* form, size_t idx, const char* msg) {
    if (!form || !msg) return;
    if (idx >= form->cap) return;

    size_t max_len = form->str_len - 1;
    strncpy(form->options[idx], msg, max_len);
    form->options[idx][max_len] = '\0';
}

// creating the input menu in the center of the screen
// using the input form struct
int create_input_menu(TUI* tui, Buffer* buffer, TUI_InputForm* form, Rect rect,
                      const char* msg) {
    if (buffer->window_cols - rect.w > 0) {
        rect.vec.x = (buffer->window_cols - rect.w) / 2;
    }
    if (buffer->window_rows - rect.h > 0) {
        rect.vec.y = (buffer->window_rows - rect.h) / 2;
    }

    draw_rect(buffer, rect);

    char* buf = malloc(BUFFER_BASE_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:create_input_menu:buf");
        return 0;
    }

    for (size_t i = 0; i < form->size; i++) {
        if (i == form->selected_option) {
            snprintf(buf, BUFFER_BASE_SIZE, "> %s %s_", form->options[i],
                     form->values[i]);
        } else {
            snprintf(buf, BUFFER_BASE_SIZE, "  %s %s", form->options[i],
                     form->values[i]);
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

    return 1;
}

int create_add_local_menu(TUI* tui, Buffer* buffer, TUI_InputForm* form) {
    if (!form) return 0;
    if (form->cap < 5) return 0;

    const size_t size = 5;
    const char*  options[] = {
        "Playlist-id: ", "Path : ", "Title : ", "Artist : ", "Album : "};
    for (size_t i = 0; i < size; i++) {
        put_in_form(form, i, options[i]);
    }
    form->size = 5;

    Rect        rect = {(Vec){0, 0}, 60, 20};
    const char* msg = "UP/DOWN: moving, SELECT to choose, LEFT to leave";

    return create_input_menu(tui, buffer, form, rect, msg);
}

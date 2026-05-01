#include "tui.h"

#include <stdio.h>

#include "buffer.h"

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

    *tui = tui_temp;
    return 1;
}

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
}

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
    size_t size =
        strlen(config->general.name) + strlen(config->general.version) + 3;
    char* headline = malloc(size * sizeof(char));
    if (!headline) {
        raise_error(ERR_MALLOC_NULL, "tui:create_header:headline");
        return 0;
    }

    snprintf(headline, size * sizeof(char), "%s %s", config->general.name,
             config->general.version);

    buffer_append_line(buffer, (Vec){0, 0}, headline);
    free(headline);

    buffer_append_line(buffer, (Vec){0, tui->header_top_border},
                       tui->separator);
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border},
                       tui->separator);
    buffer_append_line(buffer, (Vec){0, tui->header_bottom_border + 1},
                       tui->line_status);

    return 1;
}

int create_welcome(TUI* tui, Buffer* buffer, Config* config) {
    Rect rect = {(Vec){0, 0}, 60, 20};
    if (buffer->window_cols > rect.w) {
        rect.vec.x = (buffer->window_cols - rect.w) / 2;
    }
    if (buffer->window_rows > rect.h) {
        rect.vec.y = (buffer->window_rows - rect.h) / 2;
    }

    draw_rect(buffer, rect);

#define BUFFER_BASE_SIZE 128 * sizeof(char)
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
                 library->playlists[i].name);
        buffer_append_line(buffer,
                           (Vec){tui->x_playlists, tui->y_playlists + i}, buf);
    }

    return 1;
}

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

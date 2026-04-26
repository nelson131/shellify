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

    tui_temp->header_top_border = 2;
    tui_temp->header_bottom_border = *window_rows - 3;

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
    size_t top_border = buffer->window_rows / 2 - 17;
    size_t bottom_border = buffer->window_rows / 2;
    size_t left_border = buffer->window_cols / 2 - 30;
    size_t right_border = buffer->window_cols / 2 + 30;

    size_t len_floor = right_border - left_border;
    size_t len_wall = bottom_border - top_border;

    char* floor = malloc(len_floor * sizeof(char) + 1);
    if (!floor) {
        raise_error(ERR_MALLOC_NULL, "tui:create_welcome:floor");
        return 0;
    } else {
        for (size_t i = 0; i < len_floor; i++) {
            floor[i] = '-';
        }
        floor[len_floor] = '\0';
    }

    char* wall = malloc(len_wall * sizeof(char) + 1);
    if (!wall) {
        raise_error(ERR_MALLOC_NULL, "tui:create_welcome:wall");
        free(floor);
        return 0;
    } else {
        for (size_t i = 0; i < len_wall; i++) {
            wall[i] = '|';
        }
        wall[len_wall] = '\0';
    }

    buffer_append_line(buffer, (Vec){left_border, top_border}, floor);
    buffer_append_line(buffer, (Vec){left_border, bottom_border}, floor);
    buffer_append_vertical_line(buffer, (Vec){left_border, top_border}, wall);
    buffer_append_vertical_line(buffer, (Vec){right_border, top_border}, wall);

    free(floor);
    free(wall);

#define BUFFER_SIZE 128 * sizeof(char)
    char* buf = malloc(BUFFER_SIZE);
    if (!buf) {
        raise_error(ERR_MALLOC_NULL, "tui:create_welcome:buffer");
        return 0;
    }

    size_t center_sign = left_border + len_floor / 2;
    snprintf(buf, BUFFER_SIZE, "%s %s", config->general.name,
             config->general.version);
    buffer_append_line(buffer, (Vec){center_sign - strlen(buf), top_border + 1},
                       buf);

    buffer_append_line(
        buffer,
        (Vec){center_sign - strlen(config->general.desc), top_border + 3},
        config->general.desc);

    free(buffer);

    return 1;
}

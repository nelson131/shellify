#include "tui.h"

#include <stdio.h>

#include "buffer.h"

TUI* tui = NULL;

int tui_init() {
    tui = malloc(sizeof(TUI));
    if (!tui) {
        raise_error(ERR_MALLOC_NULL, "tui:init_tui:tui");
        return 0;
    }

    if (!tui->separator) {
        char* temp = malloc((window_cols + 1) * sizeof(char));
        if (!temp) {
            raise_error(ERR_MALLOC_NULL, "tui:init_tui:separator");
            return 0;
        }

        for (size_t i = 0; i < window_cols; i++) {
            temp[i] = '_';
        }

        temp[window_cols + 1] = '\0';
        tui->separator = temp;
    }

#define PREFIX_SIZE strlen(PREFIX_PLAYING)
    if (!tui->line_status) {
        size_t str_size = ((window_cols - PREFIX_SIZE) + 1) * sizeof(char);
        char*  temp = malloc(str_size);
        if (!temp) {
            raise_error(ERR_MALLOC_NULL, "tui:init_tui:now_playing");
            return 0;
        }

        tui->line_status = temp;

#define MAX_SONG_LEN 64
        if (!tui->song_name) {
            char* temp = calloc(' ', MAX_SONG_LEN * sizeof(char));
            if (!temp) {
                raise_error(ERR_MALLOC_NULL, "tui:init_tui:song_name");
                return 0;
            }

            tui->song_name = temp;
        }

        snprintf(tui->line_status, str_size, " %s%s\n", PREFIX_PLAYING,
                 tui->song_name);
    }

    tui->header_top_border = 2;
    tui->header_bottom_border = window_rows - 3;

    return 1;
}

void tui_clear() {
    if (tui->separator) {
        free(tui->separator);
    }

    if (tui->line_status) {
        free(tui->line_status);
    }

    if (tui->song_name) {
        free(tui->song_name);
    }

    if (tui) {
        free(tui);
    }
}

int create_header() {
    size_t size =
        strlen(config.general.name) + strlen(config.general.version) + 3;
    char* headline = malloc(size * sizeof(char));
    if (!headline) {
        raise_error(ERR_MALLOC_NULL, "tui:create_header:headline");
        return 0;
    }

    snprintf(headline, size * sizeof(char), "%s %s", config.general.name,
             config.general.version);

    buffer_append_line(0, 0, headline);
    free(headline);

    buffer_append_line(0, tui->header_top_border, tui->separator);
    buffer_append_line(0, tui->header_bottom_border, tui->separator);
    buffer_append_line(0, tui->header_bottom_border + 1, tui->line_status);

    return 1;
}

int operate_welcome() {
    size_t top_border = window_rows / 2 - 17;
    size_t bottom_border = window_rows / 2;
    size_t left_border = window_cols / 2 - 30;
    size_t right_border = window_cols / 2 + 30;

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
        floor[len_floor + 1] = '\0';
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
        wall[len_wall + 1] = '\0';
    }

    buffer_append_line(left_border, top_border, floor);
    buffer_append_line(left_border, bottom_border, floor);
    buffer_append_vertical_line(left_border, top_border, wall);
    buffer_append_vertical_line(right_border, top_border, wall);

    free(floor);
    free(wall);

#define BUFFER_SIZE 128 * sizeof(char)
    char* buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        raise_error(ERR_MALLOC_NULL, "tui:create_welcome:buffer");
        return 0;
    }

    size_t center_sign = left_border + len_floor / 2;
    snprintf(buffer, BUFFER_SIZE, "%s %s", config.general.name,
             config.general.version);
    buffer_append_line(center_sign - strlen(buffer), top_border + 1, buffer);

    buffer_append_line(center_sign - strlen(config.general.desc),
                       top_border + 3, config.general.desc);

    free(buffer);

    return 1;
}

#include "tui.h"

#include <stdio.h>

#include "buffer.h"

char* separator = NULL;
char* line_now_playing = NULL;
char* song_name = NULL;

int tui_init() {
    if (!separator) {
        char* temp = malloc((window_cols + 1) * sizeof(char));
        if (!temp) {
            raise_error(ERR_MALLOC_NULL, "tui:init_tui:separator");
            return 0;
        }

        for (size_t i = 0; i < window_cols; i++) {
            temp[i] = '_';
        }

        temp[window_cols + 1] = '\0';
        separator = temp;
    }

#define PREFIX_SIZE strlen(PREFIX_PLAYING)
    if (!line_now_playing) {
        size_t str_size = ((window_cols - PREFIX_SIZE) + 1) * sizeof(char);
        char*  temp = malloc(str_size);
        if (!temp) {
            raise_error(ERR_MALLOC_NULL, "tui:init_tui:now_playing");
            return 0;
        }

#define MAX_SONG_LEN 64
        if (!song_name) {
            song_name = calloc(' ', MAX_SONG_LEN * sizeof(char));
            if (!song_name) {
                raise_error(ERR_MALLOC_NULL, "tui:init_tui:song_name");
                return 0;
            }
        }

        snprintf(temp, str_size, " %s%s\n", PREFIX_PLAYING, song_name);
        line_now_playing = temp;
    }

    return 1;
}

void tui_clear() {
    if (separator) {
        free(separator);
    }

    if (line_now_playing) {
        free(line_now_playing);
    }

    if (song_name) {
        free(song_name);
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

    buffer_append_line(0, 2, separator);
    buffer_append_line(0, window_rows - 3, separator);
    buffer_append_line(0, window_rows - 2, line_now_playing);

    return 1;
}

int create_welcome() {
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
    return 1;
}

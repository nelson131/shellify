#include "tui.h"

#include <stdio.h>

#include "buffer.h"

char* separator = NULL;
char* line_now_playing = NULL;
char* song_name = NULL;

void tui_init() {
    if (!separator) {
        char* temp = malloc((window_cols + 1) * sizeof(char));
        if (!temp) {
            raise_error(ERR_MALLOC_NULL, "tui:init_tui:separator");
            return;
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
            return;
        }

        if (!song_name) {
            song_name = "";
        }

        snprintf(temp, str_size, " %s%s\n", PREFIX_PLAYING, song_name);
        line_now_playing = temp;
    }
}

void tui_clear() {
    if (separator) {
        free(separator);
    }
}

void create_header() {
    size_t size =
        strlen(config.general.name) + strlen(config.general.version) + 3;
    char* headline = malloc(size * sizeof(char));
    if (!headline) {
        raise_error(ERR_MALLOC_NULL, "tui:create_header:headline");
        return;
    }

    snprintf(headline, size * sizeof(char), "%s %s", config.general.name,
             config.general.version);

    buffer_append_line(0, 0, headline);
    free(headline);

    buffer_append_line(0, 2, separator);
    buffer_append_line(0, window_rows - 3, separator);
    buffer_append_line(0, window_rows - 2, line_now_playing);
}

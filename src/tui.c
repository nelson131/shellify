#include "tui.h"

#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

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
    buffer_append_line(0, window_rows - 2, separator);
}

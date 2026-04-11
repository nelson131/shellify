#include "config.h"

#include <stdio.h>

Config config;

void config_load() {
    char* config_path = get_config_path();

    FILE* file = fopen(config_path, "r");
    if (!file) {
        free(config_path);
        raise_error(ERR_FILE_OPENING, "config:config_load:file");
        return;
    }

    char* line = malloc(CONFIG_LINE_SIZE * sizeof(char));
    if (!line) {
        free(config_path);
        fclose(file);
        raise_error(ERR_MALLOC_NULL, "config:config_load:file");
        return;
    }

    int   has_object = 0;
    char* value = malloc(64 * sizeof(char));
    if (!value) {
        free(config_path);
        free(line);
        fclose(file);
        raise_error(ERR_MALLOC_NULL, "config:config_load:value");
        return;
    }

    while (fgets(line, CONFIG_LINE_SIZE, file) != NULL) {
        if (sscanf(line, "name=%s", value) == 1) {
            strcpy(config.general.name, value);
            continue;
        }
        if (sscanf(line, "version=%s", value) == 1) {
            strcpy(config.general.version, value);
            continue;
        }

        if (sscanf(line, "volume=%zu", &config.player.volume) == 1) continue;
        if (sscanf(line, "shuffle=%zu", &config.player.shuffle) == 1) continue;

        if (sscanf(line, "quit=%c", &config.keys.quit) == 1) continue;
        if (sscanf(line, "up=%c", &config.keys.up) == 1) continue;
        if (sscanf(line, "down=%c", &config.keys.down) == 1) continue;
        if (sscanf(line, "select=%c", &config.keys.select) == 1) continue;
    }

    free(value);
    free(line);
    free(config_path);
    fclose(file);
}

void config_save() {}

char* get_config_path() {
    const char* home = getenv("HOME");

    char* path = malloc(CONFIG_PATH_SIZE * sizeof(char));
    if (!path) {
        raise_error(ERR_MALLOC_NULL, "config:get_config_path:path");
        return NULL;
    }

    snprintf(path, CONFIG_PATH_SIZE, CONFIG_PATH, home);
    return path;
}

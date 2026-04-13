#include "config.h"

Config config;

int config_load() {
    FILE* file = get_config_file("r");
    if (!file) {
        raise_error(ERR_NULL_OBJECT, "config:config_load:file");
        return 0;
    }

    char* line = malloc(CONFIG_LINE_SIZE * sizeof(char));
    if (!line) {
        fclose(file);
        raise_error(ERR_MALLOC_NULL, "config:config_load:file");
        return 0;
    }

    int   has_object = 0;
    char* value = malloc(64 * sizeof(char));
    if (!value) {
        free(line);
        fclose(file);
        raise_error(ERR_MALLOC_NULL, "config:config_load:value");
        return 0;
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
    fclose(file);
    return 1;
}

int config_save() {
    FILE* file = get_config_file("w");
    if (!file) {
        raise_error(ERR_NULL_OBJECT, "config:config_save:file");
        return 0;
    }

    char* header = get_config_header();
    fputs(header, file);
    free(header);

    fprintf(file, "[general]\nname=%s\nversion=%s\n\n", CONFIG_APP_NAME,
            CONFIG_APP_VERSION);

    fprintf(file, "[player]\nvolume=%zu\nshuffle=%zu\n\n", config.player.volume,
            config.player.shuffle);
    fprintf(file, "[keys]\nquit=%c\nup=%c\ndown=%c\nselect=%c\n",
            config.keys.quit, config.keys.up, config.keys.down,
            config.keys.select);

    fclose(file);
    return 1;
}

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

FILE* get_config_file(const char* mode) {
    char* config_path = get_config_path();
    if (!config_path) {
        raise_error(ERR_MALLOC_NULL, "config:get_config_file:file");
        return NULL;
    }

    FILE* file = fopen(config_path, mode);
    if (!file) {
        raise_error(ERR_FILE_OPENING, "config:get_config:file:file");
        return NULL;
    }
    free(config_path);

    return file;
}

char* get_config_header() {
    time_t    t = time(NULL);
    struct tm tm = *localtime(&t);

#define STR_SIZE 128 * sizeof(char)
    char* str = malloc(STR_SIZE);
    if (!str) {
        raise_error(ERR_MALLOC_NULL, "config:get_config_header:str");
        return NULL;
    }

    snprintf(str, STR_SIZE, "# Config last time saving %d-%d-%d %d:%d:%d\n",
             tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
             tm.tm_sec);

    return str;
}

#include "config.h"

Config config;

int config_load(Config** config) {
    Config* temp = malloc(sizeof(Config));
    if (!temp) {
        raise_error(ERR_MALLOC_NULL, "config:config_load:temp");
        return 0;
    }

    FILE* file = get_config_file("r");
    if (!file) {
        free(temp);
        raise_error(ERR_NULL_OBJECT, "config:config_load:file");
        return 0;
    }

    char* line = malloc(CONFIG_LINE_SIZE * sizeof(char));
    if (!line) {
        fclose(file);
        free(temp);
        raise_error(ERR_MALLOC_NULL, "config:config_load:file");
        return 0;
    }

    int has_object = 0;
    strcpy(temp->general.name, CONFIG_APP_NAME);
    strcpy(temp->general.version, CONFIG_APP_VERSION);
    while (fgets(line, CONFIG_LINE_SIZE, file) != NULL) {
        if (strncmp(line, "desc=", 5) == 0) {
            char* ptr = line + 5;
            ptr[strcspn(ptr, "\r\n")] = 0;

            strncpy(temp->general.desc, ptr, sizeof(temp->general.desc) - 1);
            temp->general.desc[sizeof(temp->general.desc) - 1] = '\0';
            continue;
        }

        if (sscanf(line, "volume=%zu", &temp->player.volume) == 1) continue;
        if (sscanf(line, "shuffle=%zu", &temp->player.shuffle) == 1) continue;

        if (sscanf(line, "quit=%c", &temp->keys.quit) == 1) continue;
        if (sscanf(line, "up=%c", &temp->keys.up) == 1) continue;
        if (sscanf(line, "down=%c", &temp->keys.down) == 1) continue;
        if (sscanf(line, "select=%c", &temp->keys.select) == 1) continue;
    }

    free(line);
    fclose(file);
    *config = temp;
    return 1;
}

int config_save(Config* config) {
    FILE* file = get_config_file("w");
    if (!file) {
        raise_error(ERR_NULL_OBJECT, "config:config_save:file");
        return 0;
    }

    char* header = get_config_header();
    fputs(header, file);
    free(header);

    fprintf(file, "[general]\ndesc=%s\n\n", config->general.desc);

    fprintf(file, "[player]\nvolume=%zu\nshuffle=%zu\n\n",
            config->player.volume, config->player.shuffle);
    fprintf(file, "[keys]\nquit=%c\nup=%c\ndown=%c\nselect=%c\n",
            config->keys.quit, config->keys.up, config->keys.down,
            config->keys.select);

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
        free(config_path);
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
             tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
             tm.tm_sec);

    return str;
}

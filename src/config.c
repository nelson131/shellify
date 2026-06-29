#include "config.h"

int config_load(Config** config) {
    Config* temp = malloc(sizeof(Config));
    if (!temp) {
        errlog(ERR_MALLOC_NULL, "config:config_load:temp");
        return 0;
    }

    config_default(temp);

    FILE* file = get_config_file("r");
    if (!file) {
        config_save(temp);
        file = get_config_file("r");
        if (!file) {
            free(temp);
            errlog(ERR_NULL_OBJECT, "config:config_load:file");
            return 0;
        }
    }

    char* line = malloc(CONFIG_LINE_SIZE * sizeof(char));
    if (!line) {
        fclose(file);
        free(temp);
        errlog(ERR_MALLOC_NULL, "config:config_load:file");
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
        if (sscanf(line, "logging=%zu", &temp->general.logging) == 1) continue;
        if (sscanf(line, "usleep=%zu", &temp->general.usleep) == 1) continue;

        if (sscanf(line, "volume=%f", &temp->player.volume) == 1) continue;
        if (sscanf(line, "shuffle=%zu", &temp->player.shuffle) == 1) continue;

        if (sscanf(line, "quit=%c", &temp->keys.quit) == 1) continue;
        if (sscanf(line, "super=%c", &temp->keys.super) == 1) continue;
        if (sscanf(line, "select=%c", &temp->keys.select) == 1) continue;
        if (sscanf(line, "pause=%c", &temp->keys.pause) == 1) continue;
        if (sscanf(line, "inc=%c", &temp->keys.inc) == 1) continue;
        if (sscanf(line, "dec=%c", &temp->keys.dec) == 1) continue;
        if (sscanf(line, "add=%c", &temp->keys.add) == 1) continue;
        if (sscanf(line, "remove=%c", &temp->keys.remove) == 1) continue;
        if (sscanf(line, "song=%c", &temp->keys.song) == 1) continue;
        if (sscanf(line, "playlist=%c", &temp->keys.playlist) == 1) continue;
        if (sscanf(line, "shuffle=%c", &temp->keys.shuffle) == 1) continue;
        if (sscanf(line, "dashboard=%c", &temp->keys.dashboard) == 1) continue;
    }

    free(line);
    fclose(file);
    *config = temp;
    slog(INFO, "config has been loaded successfully");
    return 1;
}

void config_setup() {
    const char* home = getenv("HOME");

    char* buf = malloc(CONFIG_PATH_SIZE * sizeof(char));
    if (!buf) return;
    snprintf(buf, CONFIG_PATH_SIZE, CONFIGS_DIR, home);

    struct stat statbuf;
    if (stat(buf, &statbuf) != 0) {
        mkdir(buf, 0755);
    } else {
        free(buf);
        return;
    }

    snprintf(buf, CONFIG_PATH_SIZE, CONFIG_DB_PATH, home);
    FILE* file = fopen(buf, "w");
    if (file) {
        fclose(file);
    }

    free(buf);
}

int config_save(Config* config) {
    FILE* file = get_config_file("w");
    if (!file) {
        errlog(ERR_NULL_OBJECT, "config:config_save:file");
        return 0;
    }

    char* header = get_config_header();
    fputs(header, file);
    free(header);

    fprintf(file, "[general]\ndesc=%s\nlogging=%zu\nusleep=%zu\n\n",
            config->general.desc, config->general.logging,
            config->general.usleep);

    fprintf(file, "[player]\nvolume=%f\nshuffle=%zu\n\n", config->player.volume,
            config->player.shuffle);
    fprintf(
        file,
        "[keys]\nquit=%c\nsuper=%c\nselect=%c\npause=%c\ninc=%c\ndec=%c\nadd=%"
        "c\nremove=%c\nsong=%"
        "c\nplaylist=%c\nshuffle=%c\ndashboard=%c\n",
        config->keys.quit, config->keys.super, config->keys.select,
        config->keys.pause, config->keys.inc, config->keys.dec,
        config->keys.add, config->keys.remove, config->keys.song,
        config->keys.playlist, config->keys.shuffle, config->keys.dashboard);

    fclose(file);
    slog(INFO, "config has been saved successfully");
    return 1;
}

void config_default(Config* config) {
    if (!config) return;

    strcpy(config->general.name, CONFIG_APP_NAME);
    strcpy(config->general.version, CONFIG_APP_VERSION);
    strcpy(config->general.desc, CONFIG_APP_DESC);
    config->general.logging = CONFIG_DEF_LOGGING;
    config->general.usleep = CONFIG_DEF_USLEEP;

    config->player.volume = CONFIG_DEF_VOLUME;
    config->player.shuffle = CONFIG_DEF_SHUFFLE_VALUE;

    config->keys.quit = CONFIG_DEF_QUIT;
    config->keys.super = CONFIG_DEF_SUPER;
    config->keys.select = CONFIG_DEF_SELECT;
    config->keys.pause = CONFIG_DEF_PAUSE;
    config->keys.inc = CONFIG_DEF_INCREASE;
    config->keys.dec = CONFIG_DEF_DECREASE;
    config->keys.add = CONFIG_DEF_ADD;
    config->keys.remove = CONFIG_DEF_REMOVE;
    config->keys.song = CONFIG_DEF_SONG;
    config->keys.playlist = CONFIG_DEF_PLAYLIST;
    config->keys.shuffle = CONFIG_DEF_SHUFFLE;
    config->keys.dashboard = CONFIG_DEF_DASHBOARD;
}

char* get_config_path() {
    const char* home = getenv("HOME");

    char* path = malloc(CONFIG_PATH_SIZE * sizeof(char));
    if (!path) {
        errlog(ERR_MALLOC_NULL, "config:get_config_path:path");
        return NULL;
    }

    snprintf(path, CONFIG_PATH_SIZE, CONFIG_PATH, home);
    return path;
}

FILE* get_config_file(const char* mode) {
    char* config_path = get_config_path();
    if (!config_path) {
        errlog(ERR_MALLOC_NULL, "config:get_config_file:file");
        return NULL;
    }

    FILE* file = fopen(config_path, mode);
    if (!file) {
        errlog(ERR_FILE_OPENING, "config:get_config:file:file");
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
        errlog(ERR_MALLOC_NULL, "config:get_config_header:str");
        return NULL;
    }

    snprintf(str, STR_SIZE, "# Config last time saving %d-%d-%d %d:%d:%d\n",
             tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
             tm.tm_sec);

    return str;
}

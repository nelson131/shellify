#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "logger.h"
#include "sys/stat.h"

#define CONFIG_PATH "%s/.config/shellify/config"
#define CONFIG_DB_PATH "%s/.config/shellify/shellify.db"
#define CONFIGS_DIR "%s/.config/shellify"
#define CONFIG_GENERAL_SIZE 16
#define CONFIG_PATH_SIZE 256
#define CONFIG_LINE_SIZE 128

#define CONFIG_APP_NAME "shellify"
#define CONFIG_APP_VERSION "v2.1.0"
#define CONFIG_APP_DESC "terminal based audio player"

#define CONFIG_DEF_LOGGING 0
#define CONFIG_DEF_VOLUME 50
#define CONFIG_DEF_SHUFFLE_VALUE 0
#define CONFIG_DEF_QUIT 'q'
#define CONFIG_DEF_SUPER 'x'
#define CONFIG_DEF_SELECT 'e'
#define CONFIG_DEF_PAUSE 'f'
#define CONFIG_DEF_INCREASE ']'
#define CONFIG_DEF_DECREASE '['
#define CONFIG_DEF_ADD 'a'
#define CONFIG_DEF_REMOVE 'r'
#define CONFIG_DEF_SONG 's'
#define CONFIG_DEF_PLAYLIST 'p'
#define CONFIG_DEF_SHUFFLE 'u'
#define CONFIG_DEF_DASHBOARD 'd'

typedef struct cfg_general {
    char   name[CONFIG_GENERAL_SIZE];
    char   version[CONFIG_GENERAL_SIZE];
    char   desc[CONFIG_PATH_SIZE];
    size_t logging;
} cfg_general;

typedef struct cfg_player {
    float  volume;
    size_t shuffle;
} cfg_player;

typedef struct cfg_keys {
    char quit;
    char super;
    char select;
    char pause;

    char inc;
    char dec;

    char add;
    char remove;
    char song;
    char playlist;
    char shuffle;
    char dashboard;
} cfg_keys;

typedef struct Config {
    cfg_general general;
    cfg_player  player;
    cfg_keys    keys;
} Config;

int  config_load(Config** config);
void config_setup();
int  config_save(Config* config);

void config_default(Config* config);

char* get_config_path();
FILE* get_config_file(const char* mode);
char* get_config_header();

#endif

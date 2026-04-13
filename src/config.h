#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "error_handler.h"

#define CONFIG_PATH "%s/.config/shellify/config"
#define CONFIG_PATH_SIZE 256
#define CONFIG_LINE_SIZE 128

#define CONFIG_APP_NAME "shellify"
#define CONFIG_APP_VERSION "v0.1.0"

typedef enum cfg_category { GENERAL, PLAYER, KEYS } cfg_category;

typedef struct cfg_general {
    char name[16];
    char version[16];
} cfg_general;

typedef struct cfg_player {
    size_t volume;
    size_t shuffle;
} cfg_player;

typedef struct cfg_keys {
    char quit;
    char up;
    char down;
    char select;
} cfg_keys;

typedef struct Config {
    cfg_general general;
    cfg_player  player;
    cfg_keys    keys;
} Config;

extern Config config;

void config_load();
void config_save();

char* get_config_path();
FILE* get_config_file(const char* mode);
char* get_config_header();

#endif

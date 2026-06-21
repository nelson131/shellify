#ifndef STORAGE_H
#define STORAGE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "db_handler.h"
#include "dl_queue.h"
#include "library.h"
#include "logger.h"

#define STG_DIR_NAME "shellify"
#define MUSIC_DIR "%s/Music"
#define STG_DIR "%s/Music/%s"

typedef struct Storage {
    Library* lib;
    sqlite3* db;
    DLQueue* dlq;
} Storage;

// >>> main funcs
Storage* stg_init();
void     stg_close(Storage* stg);

int stg_load(Storage* stg);

// >>> songs
int stg_add_sng(Storage* stg, Song* sng);
int stg_rem_sng_abs(Storage* stg, Song* sng);
int stg_rem_sng(Storage* stg, Song* sng, Playlist* plist);

// >>> playlists
int stg_add_plist(Storage* stg, Playlist* plist);
int stg_rem_plist(Storage* stg, Playlist* plist);

// >>> connections
int stg_conn(Storage* stg, Song* sng, Playlist* plist);

// >>> utils
void  init_music_dir();
char* get_music_dir();

#endif

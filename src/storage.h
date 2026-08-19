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
#include "import_pl.h"
#include "library.h"
#include "logger.h"
#include "search.h"

#define STG_DIR_NAME "shellify"
#define MUSIC_DIR "%s/Music"
#define STG_DIR "%s/Music/%s"

typedef struct Storage {
    Library*    lib;
    sqlite3*    db;
    DLQueue*    dlq;
    SearchCore* sr_core;
    ImportData* import_data;
} Storage;

// >>> main funcs
Storage* stg_init();
void     stg_close(Storage* stg);

int stg_load(Storage* stg);

// >>> songs
int  stg_add_sng(Storage* stg, Song* sng);
int  stg_rem_sng_abs(Storage* stg, Song* sng);
int  stg_rem_sng(Storage* stg, Song* sng, Playlist* plist);
int  stg_update_sng(Storage* stg, Song* sng, const char* title,
                    const char* artist, const char* album);
void stg_move_sng(Storage* stg, Playlist* plist, size_t from, size_t to);

// >>> playlists
int stg_add_plist(Storage* stg, Playlist* plist);
int stg_rem_plist(Storage* stg, Playlist* plist);

// >>> connections
int stg_conn(Storage* stg, Song* sng, Playlist* plist);

// >>> dlq
int stg_clear_dlq(Storage* stg);
int stg_add_dlq_task(Storage* stg, DLTask* task);

// >>> utils
void  init_music_dir();
char* get_music_dir();

#endif

#ifndef STORAGE_H
#define STORAGE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "db_handler.h"
#include "library.h"
#include "logger.h"

typedef struct Storage {
    Library* lib;
    sqlite3* db;
} Storage;

Storage* stg_init();
void     stg_close(Storage* stg);

int stg_load(Storage* stg);

int stg_add_sng(Storage* stg, Song* sng);
int stg_rem_sng();

int stg_add_plist(Storage* stg, Playlist* plist);
int stg_rem_plist(Storage* stg, Playlist* plist);

int stg_conn(Storage* stg, Song* sng, Playlist* plist);

#endif

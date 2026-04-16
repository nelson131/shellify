#ifndef DB_HANDLER_H
#define DB_HANDLER_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_handler.h"

#define DB_FILE_PATH "%s/.config/shellify/shellify.db"

sqlite3* db_init();
int      db_close(sqlite3* db);

int           db_execute(sqlite3* db, const char* query);
sqlite3_stmt* db_prepare(sqlite3* db, const char* query);

char* get_db_file_path();

#endif

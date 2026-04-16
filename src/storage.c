#include "storage.h"

#include "db_handler.h"

sqlite3* storage_init() {
    sqlite3* db = db_init();

    const char* query =
        "CREATE TABLE IF NOT EXISTS songs(id UNSIGNED INTEGER PRIMARY KEY, "
        "path TEXT UNIQUE, title TEXT, artist TEXT, album TEXT, duration "
        "INTEGER, mtime INTEGER)";

    if (!db_execute(db, query)) {
        raise_error(ERR_SQLITE_FAILED, "storage:init:execute:songs");
        return NULL;
    }

    query =
        "CREATE TABLE IF NOT EXISTS playlists(id UNSIGNED INTEGER PRIMARY KEY, "
        "name TEXT, songs TEXT)";

    if (!db_execute(db, query)) {
        raise_error(ERR_SQLITE_FAILED, "storage:init:execute:playlists");
        return NULL;
    }

    return db;
}

int storage_close(sqlite3** db) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:close:db");
        return 0;
    }

    return db_close(*db);
}

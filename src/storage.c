#include "storage.h"

#include "db_handler.h"

int storage_init(sqlite3** db) {
    sqlite3* temp = db_init();

    const char* query =
        "CREATE TABLE IF NOT EXISTS songs(id UNSIGNED INTEGER PRIMARY KEY, "
        "path TEXT UNIQUE, title TEXT, artist TEXT, album TEXT, duration "
        "INTEGER, mtime INTEGER)";

    if (!db_execute(temp, query)) {
        raise_error(ERR_SQLITE_FAILED, "storage:init:execute:songs");
        return 0;
    }

    query =
        "CREATE TABLE IF NOT EXISTS playlists(id UNSIGNED INTEGER PRIMARY KEY, "
        "name TEXT, songs TEXT)";

    if (!db_execute(temp, query)) {
        raise_error(ERR_SQLITE_FAILED, "storage:init:execute:playlists");
        return 0;
    }

    *db = temp;
    return 1;
}

int storage_close(sqlite3** db) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:close:db");
        return 0;
    }

    return db_close(*db);
}

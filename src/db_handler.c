#include "db_handler.h"

sqlite3* db_init() {
    char* file_path = get_db_file_path();
    if (!file_path) {
        raise_error(ERR_NULL_OBJECT, "db_handler:init:file_path");
        return NULL;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(file_path, &db)) {
        raise_error(ERR_SQLITE_OPEN, "db_handler:init:open");
        free(file_path);
        return NULL;
    }

    return db;
}

int db_close(sqlite3* db) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "db_handler:close:db");
        return 0;
    }

    sqlite3_close(db);
    return 1;
}

int db_execute(sqlite3* db, const char* query) {
    if (!query) {
        raise_error(ERR_EMPTY_OBJECT, "db_hander:execute:query");
        return 0;
    }

    char* err_msg = NULL;
    int   rc = sqlite3_exec(db, query, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        raise_error(ERR_SQLITE_FAILED, err_msg);
        sqlite3_free(err_msg);
        return 0;
    }

    return 1;
}

char* get_db_file_path() {
    const char* home_path = getenv("HOME");

#define MAX_PATH_LEN 256 * sizeof(char)
    char* path = malloc(MAX_PATH_LEN);
    if (!path) {
        raise_error(ERR_MALLOC_NULL, "db_handler:get_db_file_path:path");
        return NULL;
    }

    snprintf(path, MAX_PATH_LEN, DB_FILE_PATH, home_path);
    return path;
}

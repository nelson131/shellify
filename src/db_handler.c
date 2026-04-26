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

    free(file_path);
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
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "db_handler:execute:db");
        return 0;
    }

    if (!query) {
        raise_error(ERR_EMPTY_OBJECT, "db_handler:execute:query");
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

sqlite3_stmt* db_prepare(sqlite3* db, const char* query) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "db_handler:prepare:db");
        return NULL;
    }

    if (!query) {
        raise_error(ERR_EMPTY_OBJECT, "db_handler:prepare:query");
        return NULL;
    }

    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare16_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        raise_error(ERR_SQLITE_FAILED, "db_handler:prepare:sqlite");
        return NULL;
    }

    return stmt;
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

#include "storage.h"

#include "library.h"
#include "logger.h"

// Initialization of storage:
// allocating the storage and a library,
// creating sql tables if they dont exist
Storage* stg_init() {
    Storage* storage = malloc(sizeof(Storage));
    if (!storage) {
        errlog(ERR_MALLOC_NULL, "stg:init:stg");
        return NULL;
    }

#define BASE_SONGS_CAP 32
#define BASE_PLISTS_CAP 4
    storage->lib = lib_create(BASE_SONGS_CAP, BASE_PLISTS_CAP);
    if (!storage->lib) {
        errlog(ERR_NULL_OBJECT, "stg:init:lib");
        free(storage);
        return NULL;
    }

    storage->db = db_init();
    if (!storage->db) {
        errlog(ERR_SQLITE_FAILED, "stg:init:temp");
        lib_clear(storage->lib);
        free(storage);
        return NULL;
    }

    const char* query =
        "CREATE TABLE IF NOT EXISTS songs(id INTEGER PRIMARY KEY, "
        "path TEXT UNIQUE, title TEXT, artist TEXT, album TEXT, duration "
        "INTEGER, mtime INTEGER)";

    if (!db_execute(storage->db, query)) {
        goto db_error;
    }

    query =
        "CREATE TABLE IF NOT EXISTS playlists(id INTEGER PRIMARY KEY, "
        "name TEXT)";

    if (!db_execute(storage->db, query)) {
        goto db_error;
    }

    query =
        "CREATE TABLE IF NOT EXISTS playlist_songs(playlist_id INTEGER, "
        "song_id INTEGER, position INTEGER, FOREIGN KEY(playlist_id) "
        "REFERENCES playlists(id), FOREIGN KEY(song_id) REFERENCES songs(id) "
        "ON DELETE CASCADE)";

    if (!db_execute(storage->db, query)) {
        goto db_error;
    }

    storage->dlq = dlq_init();

    init_music_dir();

    slog(INFO, "storage has been init");
    return storage;

db_error:
    errlog(ERR_SQLITE_FAILED, "stg:init:execute");
    lib_clear(storage->lib);
    db_close(storage->db);
    free(storage);
    return NULL;
}

// closing the storage:
// clearing library and closing sql db
void stg_close(Storage* stg) {
    if (!stg) {
        errlog(ERR_NULL_OBJECT, "stg:close:stg");
        return;
    }

    if (stg->lib) {
        lib_clear(stg->lib);
        free(stg->lib);
    }

    if (stg->db) {
        db_close(stg->db);
    }

    if (stg->dlq) {
        dlq_close(stg->dlq);
    }

    slog(INFO, "storage has been closed");
}

// loading the storage:
// loading into the memory data from
// sql tables like songs, playlists, playlist_songs
int stg_load(Storage* stg) {
    if (!stg) {
        errlog(ERR_NULL_OBJECT, "stg:load:stg");
        return 0;
    }

    sqlite3_stmt* stmt = NULL;

    // >>> INSERTING SONGS
    const char* query =
        "SELECT id, path, title, artist, album, duration, mtime FROM songs";
    stmt = db_prepare(stg->db, query);
    if (!stmt) goto stmt_error;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        lib_new_sng(stg->lib, sqlite3_column_int64(stmt, 0),
                    (const char*)sqlite3_column_text(stmt, 1),
                    (const char*)sqlite3_column_text(stmt, 2),
                    (const char*)sqlite3_column_text(stmt, 3),
                    (const char*)sqlite3_column_text(stmt, 4),
                    sqlite3_column_int64(stmt, 5),
                    (time_t)sqlite3_column_int64(stmt, 6));
    }

    sqlite3_finalize(stmt);
    slog(INFO, "songs loaded into the storage");

    // >>> INSERTING PLAYLISTS

    query = "SELECT id, name FROM playlists";
    stmt = db_prepare(stg->db, query);
    if (!stmt) goto stmt_error;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        lib_new_plist(stg->lib, sqlite3_column_int64(stmt, 0),
                      (const char*)sqlite3_column_text(stmt, 1),
                      BASE_PLISTS_CAP);
    }

    sqlite3_finalize(stmt);
    slog(INFO, "playlists loaded into the storage");

    // >>> CREATING CONNECTION BETWEEN SONGS AND PLAYLISTS
    query =
        "SELECT song_id FROM playlist_songs WHERE playlist_id = ? ORDER BY "
        "position";
    stmt = db_prepare(stg->db, query);
    if (!stmt) goto stmt_error;

    for (size_t i = 0; i < stg->lib->playlist_count; i++) {
        Playlist* playlist = stg->lib->playlists[i];

        sqlite3_reset(stmt);
        bind_int(stmt, 1, playlist->id);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            size_t id = sqlite3_column_int64(stmt, 0);
            Song*  s = find_sng_by_id(stg->lib, id);
            if (s) {
                if (playlist->song_count + 1 >= playlist->capacity) {
                    size_t new_cap = playlist->capacity * 2;
                    Song** temp =
                        realloc(playlist->songs, new_cap * sizeof(Song*));
                    if (!temp) {
                        errlog(ERR_MALLOC_NULL, "stg:load:temp");
                        sqlite3_finalize(stmt);
                        return 0;
                    }

                    playlist->capacity = new_cap;
                    playlist->songs = temp;
                }

                playlist->songs[playlist->song_count++] = s;
            } else {
                char buf[64];
                snprintf(
                    buf, sizeof(buf),
                    "failed to find song with id: %zu, in the storage loading",
                    id);
                slog(WARNING, buf);
            }
        }
    }

    sqlite3_finalize(stmt);
    slog(INFO, "connection songs->playlists loaded");

    slog(INFO, "storage data has been loaded");
    return 1;

stmt_error:
    errlog(ERR_SQLITE_FAILED, "stg:load:stmt");
    return 0;
}

// >>> SONGS

// adding the memory allocated song into
// the sql db + inserting the real song id
int stg_add_sng(Storage* stg, Song* sng) {
    if (!stg || !sng) {
        errlog(ERR_NULL_OBJECT, "stg:add_sng:args");
        return 0;
    }

    const char* query =
        "INSERT INTO songs (path, title, artist, album, duration, mtime) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = db_prepare(stg->db, query);

    bind_str(stmt, 1, sng->path);
    bind_str(stmt, 2, sng->title);
    bind_str(stmt, 3, sng->artist);
    bind_str(stmt, 4, sng->album);
    bind_int(stmt, 5, sng->duration);
    bind_time(stmt, 6, sng->time);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        errlog(ERR_SQLITE_FAILED, "stg:add_sng:failed_to_add");
        return 0;
    }

    sng->id = get_db_last_id(stg->db);

    sqlite3_finalize(stmt);
    alog(INFO, sng->path, "song has been added into the db");
    return 1;
}

// absolute (full) removing the song:
// removing it from songs and connection tables
int stg_rem_sng_abs(Storage* stg, Song* sng) {
    if (!stg || !sng) {
        errlog(ERR_NULL_OBJECT, "stg:rem_sng_abs:args");
        return 0;
    }

#define BUF_BASE_SIZE 256
    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "stg:rem_sng:abs:buf");
        return 0;
    }
    char* path = get_music_dir();
    snprintf(buf, BUF_BASE_SIZE, "%s/%s", path, sng->path);

    if (unlink(buf) != 0) {
        alog(WARNING, buf, "song file not found");
    }
    free(buf);
    free(path);

    const char*   query = "DELETE FROM songs WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(stg->db, query);
    if (!stmt) {
        errlog(ERR_SQLITE_FAILED, "stg:rem_sng_abs:stmt:songs");
        return 0;
    }

    bind_int(stmt, 1, sng->id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        errlog(ERR_SQLITE_FAILED, "stg:rem_sng_abs:step");
        return 0;
    }

    sqlite3_finalize(stmt);

    alog(INFO, sng->title, "song has been deleted from conn and db");
    return 1;
}

// local removing the song:
// removing it only from connection table!!!!!
int stg_rem_sng(Storage* stg, Song* sng, Playlist* plist) {
    if (!stg || !sng || !plist) {
        errlog(ERR_NULL_OBJECT, "stg:rem_song:args");
        return 0;
    }

    const char* query =
        "DELETE from playlist_songs WHERE playlist_id = ? AND song_id = ?";
    sqlite3_stmt* stmt = db_prepare(stg->db, query);
    if (!stmt) {
        errlog(ERR_SQLITE_FAILED, "stg:rem_song:stmt");
        return 0;
    }

    bind_int(stmt, 1, plist->id);
    bind_int(stmt, 2, sng->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        errlog(ERR_SQLITE_FAILED, "stg:rem_song:step");
        return 0;
    }

    sqlite3_finalize(stmt);

    alog(INFO, sng->title, "song has been removed from the playlist");
    return 1;
}

// >>> PLAYLISTS

// adding the memory allocated playlist
// into the sql db
int stg_add_plist(Storage* stg, Playlist* plist) {
    if (!stg || !plist) {
        errlog(ERR_NULL_OBJECT, "stg:add_plist:args");
        return 0;
    }

    const char*   query = "INSERT INTO playlists (name) VALUES (?)";
    sqlite3_stmt* stmt = db_prepare(stg->db, query);

    bind_str(stmt, 1, plist->name);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        errlog(ERR_SQLITE_FAILED, "stg:add_plist:stmt");
        return 0;
    }

    plist->id = get_db_last_id(stg->db);

    sqlite3_finalize(stmt);
    alog(INFO, plist->name, "playlist has been added into the db");
    return 1;
}

// removing the playlist from sql db
int stg_rem_plist(Storage* stg, Playlist* plist) {
    if (!stg || !plist) {
        errlog(ERR_NULL_OBJECT, "stg:rem_plist:args");
        return 0;
    }

    const char*   query = "DELETE FROM playlist_songs WHERE playlist_id = ?";
    sqlite3_stmt* stmt = db_prepare(stg->db, query);
    if (!stmt) {
        errlog(ERR_SQLITE_FAILED, "stg:rem_plist:stmt:conn");
        return 0;
    }

    bind_int(stmt, 1, plist->id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        errlog(ERR_SQLITE_FAILED, "stg:rem_plist:step:conn");
        return 0;
    }
    sqlite3_finalize(stmt);

    query = "DELETE FROM playlists WHERE id = ?";
    stmt = db_prepare(stg->db, query);
    if (!stmt) {
        errlog(ERR_SQLITE_FAILED, "stg:rem_plist:stmt:plist");
        return 0;
    }

    bind_int(stmt, 1, plist->id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        errlog(ERR_SQLITE_FAILED, "stg:rem_plist:step:plist");
        return 0;
    }
    sqlite3_finalize(stmt);

    alog(INFO, plist->name, "playlist has been deleted from conn and db");
    return 1;
}

// >>> CONNECTION

// creating connection between song and playlist:
// assigning a song to his playlist
int stg_conn(Storage* stg, Song* sng, Playlist* plist) {
    if (!stg || !sng || !plist) {
        errlog(ERR_NULL_OBJECT, "stg:conn:args");
        return 0;
    }

    const char* query =
        "INSERT INTO playlist_songs (playlist_id, song_id, position) VALUES "
        "(?, ?, ?)";
    sqlite3_stmt* stmt = db_prepare(stg->db, query);

    bind_int(stmt, 1, plist->id);
    bind_int(stmt, 2, sng->id);
    bind_int(stmt, 3, plist->song_count);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        alog(ERROR, sqlite3_errmsg(stg->db), "stg:conn:stmt");
        return 0;
    }

    sqlite3_finalize(stmt);

    if (plist->song_count + 1 >= plist->capacity) {
        size_t new_cap = plist->capacity * 2;
        Song** temp = realloc(plist->songs, new_cap * sizeof(Song*));
        if (!temp) {
            errlog(ERR_MALLOC_NULL, "stg:conn:temp");
            return 0;
        }

        plist->capacity = new_cap;
        plist->songs = temp;
    }

    plist->songs[plist->song_count++] = sng;
    alog(INFO, sng->path, "created connection with a playlist");
    alog(INFO, plist->name, "created connection with a song");
    return 1;
}

// >>> utils

// creating Music/ and shellify/ if they dont exist
void init_music_dir() {
    const char* home = getenv("HOME");
    if (!home) {
        errlog(ERR_NULL_OBJECT, "stg:init_dir:home");
        return;
    }

    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "stg:init_dir:buf");
        return;
    }

    struct stat statbuf;

    snprintf(buf, BUF_BASE_SIZE, MUSIC_DIR, home);
    if (stat(buf, &statbuf) != 0) {
        if (mkdir(buf, 0755) != 0) {
            errlog(FAILED, "stg:init_dir:failed_dir1");
            free(buf);
            return;
        }

        slog(INFO, "music dir has been created");
    }

    snprintf(buf, BUF_BASE_SIZE, STG_DIR, home, STG_DIR_NAME);
    if (stat(buf, &statbuf) != 0) {
        if (mkdir(buf, 0755) != 0) {
            errlog(FAILED, "stg:init_dir:failed_dir2");
            free(buf);
            return;
        }

        slog(INFO, "stg dir has been created");
    }

    free(buf);
}

char* get_music_dir() {
    const char* home = getenv("HOME");
    if (!home) {
        errlog(ERR_NULL_OBJECT, "stg:get_dir:home");
        return NULL;
    }

    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) {
        errlog(ERR_MALLOC_NULL, "stg:get_dir:buf");
        return NULL;
    }

    snprintf(buf, BUF_BASE_SIZE, STG_DIR, home, STG_DIR_NAME);
    return buf;
}

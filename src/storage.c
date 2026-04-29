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
        db_close(*db);
        return 0;
    }

    query =
        "CREATE TABLE IF NOT EXISTS playlists(id UNSIGNED INTEGER PRIMARY KEY, "
        "name TEXT)";

    if (!db_execute(temp, query)) {
        raise_error(ERR_SQLITE_FAILED, "storage:init:execute:playlists");
        db_close(*db);
        return 0;
    }

    query =
        "CREATE TABLE IF NOT EXISTS playlist_songs(playlist_id INTEGER, "
        "song_id INTEGER, position INTEGER, FOREIGN KEY(playlist_id) "
        "REFERENCES playlists(id), FOREIGN KEY(song_id) REFERENCES songs(id))";

    if (!db_execute(temp, query)) {
        raise_error(ERR_SQLITE_FAILED, "storage:init:execute:playlist_songs");
        db_close(*db);
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

int storage_load(sqlite3* db, Library** library) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:load:db");
        return 0;
    }

    Library* temp = malloc(sizeof(Library));
    if (!temp) {
        raise_error(ERR_MALLOC_NULL, "storage:load:library");
        return 0;
    }

    sqlite3_stmt* stmt = NULL;
    const char*   query =
        "SELECT id, path, title, artist, album, duration, mtime FROM songs";

    stmt = db_prepare(db, query);
    if (!stmt) {
        raise_error(ERR_SQLITE_FAILED, "storage:load:prepare:stmt");
        library_clear(temp);
        return 0;
    }

#define SONGS_BASE_CAPACITY 16
    size_t capacity = SONGS_BASE_CAPACITY;
    temp->song_count = 0;
    temp->songs = malloc(capacity * sizeof(Song));
    if (!temp->songs) {
        raise_error(ERR_MALLOC_NULL, "storage:load:songs");
        sqlite3_finalize(stmt);
        library_clear(temp);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (temp->song_count + 1 > capacity) {
            capacity *= 2;
            Song* realloc_temp = realloc(temp->songs, capacity * sizeof(Song));
            temp->songs = realloc_temp;
        }

        Song* s = &temp->songs[temp->song_count];
        s->id = sqlite3_column_int64(stmt, 0);
        s->path = strdup((const char*)sqlite3_column_text(stmt, 1));
        s->title = strdup((const char*)sqlite3_column_text(stmt, 2));
        s->artist = strdup((const char*)sqlite3_column_text(stmt, 3));
        s->album = strdup((const char*)sqlite3_column_text(stmt, 4));
        s->duration = sqlite3_column_int64(stmt, 5);
        s->time = (time_t)sqlite3_column_int64(stmt, 6);

        temp->song_count++;
    }

    sqlite3_finalize(stmt);

    query = "SELECT id, name FROM playlists";
    stmt = db_prepare(db, query);
    if (!stmt) {
        raise_error(ERR_SQLITE_FAILED, "storage:load:stmt");
        library_clear(temp);
        return 0;
    };

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        temp->playlists = realloc(
            temp->playlists, sizeof(Playlist) * (temp->playlist_count + 1));
        Playlist* p = &temp->playlists[temp->playlist_count];

        p->id = sqlite3_column_int64(stmt, 0);
        p->name = strdup((const char*)sqlite3_column_text(stmt, 1));
        p->songs = NULL;
        p->song_count = 0;

        temp->playlist_count++;
    }
    sqlite3_finalize(stmt);

    query =
        "SELECT song_id FROM playlist_songs WHERE playlist_id = ? ORDER BY "
        "position";
    stmt = db_prepare(db, query);
    if (!stmt) {
        raise_error(ERR_SQLITE_FAILED, "storage:load:stmt");
        library_clear(temp);
        free(temp);
        return 0;
    }

    for (size_t i = 0; i < temp->playlist_count; i++) {
        Playlist* playlist = &temp->playlists[i];

        sqlite3_reset(stmt);
        bind_int(stmt, 1, playlist->id);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            size_t s_id = sqlite3_column_int64(stmt, 0);
            Song*  target = find_song_by_id(temp, s_id);

            if (target) {
                playlist->songs =
                    realloc(playlist->songs,
                            sizeof(Song*) * (playlist->song_count + 1));
                playlist->songs[playlist->song_count] = target;
                playlist->song_count++;
            }
        }
    }
    sqlite3_finalize(stmt);

    *library = temp;
    return 1;
}

Song* find_song_by_id(Library* library, size_t id) {
    if (!library) {
        raise_error(ERR_NULL_OBJECT, "storage:find_song_by_id:library");
        return NULL;
    }

    for (size_t i = 0; i < library->song_count; i++) {
        if (library->songs[i].id == id) {
            return &library->songs[i];
        }
    }

    raise_error(ERR_SONG_NOT_FOUND, "storage:find_song_by_id");
    return NULL;
}

int storage_add_song(Library* library, const char* path, const char* title,
                     const char* artist, const char* album, size_t duration) {
    if (!library) {
        raise_error(ERR_NULL_OBJECT, "storage:add_song:library");
        return 0;
    }

    if (!path || !title || !artist || !album) {
        raise_error(ERR_NULL_OBJECT, "storage:add_song:args");
        return 0;
    }

    Song* song = &library->songs[library->song_count];
    song->path = copy_str(path);
    song->title = copy_str(title);
    song->artist = copy_str(artist);
    song->album = copy_str(album);
    song->duration = duration;
    time(&song->time);

    return 1;
}

void library_clear(Library* library) {
    if (!library) return;

    for (size_t i = 0; i < library->playlist_count; i++) {
        playlist_clear(&library->playlists[i]);
    }

    for (size_t i = 0; i < library->song_count; i++) {
        song_clear(&library->songs[i]);
    }

    library->song_count = 0;
    library->playlist_count = 0;
}

void playlist_clear(Playlist* playlist) {
    if (!playlist) return;

    playlist->id = 0;
    playlist->song_count = 0;
    playlist->songs = NULL;

    if (playlist->name) {
        free(playlist->name);
    }

    free(playlist);
}

void song_clear(Song* song) {
    if (!song) return;

    if (song->path) {
        free(song->path);
    }

    if (song->title) {
        free(song->title);
    }

    if (song->artist) {
        free(song->artist);
    }

    if (song->album) {
        free(song->album);
    }

    free(song);
}

char* copy_str(const char* str) {
    if (!str) {
        raise_error(ERR_NULL_OBJECT, "storage:copy_str:str");
        return NULL;
    }

    char* s = malloc((strlen(str) + 1) * sizeof(char));
    if (!s) {
        raise_error(ERR_MALLOC_NULL, "storage:copy_str:s");
        return NULL;
    }

    strcpy(s, str);
    return s;
}

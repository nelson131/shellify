#include "storage.h"

#include "db_handler.h"
#include "error_handler.h"

// initialization of sqlite and creating shellify tables
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

// clearing the library and returning the status of sqlite closign
int storage_close(sqlite3** db, Library** library) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:close:db");
        return 0;
    }

    library_clear(*library);
    free(*library);
    *library = NULL;

    return db_close(*db);
}

// loading the library: creating the library struct in the heap,
// and loading songs, playlists and connection beetween songs and
// playlists into the stack/heap
int storage_load(sqlite3* db, Library** library) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:load:db");
        return 0;
    }

    Library* temp = calloc(1, sizeof(Library));
    if (!temp) {
        raise_error(ERR_MALLOC_NULL, "storage:load:library");
        return 0;
    }

    sqlite3_stmt* stmt = NULL;

    // INSERTING SONGS
    const char* query =
        "SELECT id, path, title, artist, album, duration, mtime FROM songs";
    stmt = db_prepare(db, query);
    if (!stmt) {
        raise_error(ERR_SQLITE_FAILED, "storage:load:prepare:stmt");
        library_clear(temp);
        free(temp);
        return 0;
    }

#define SONGS_BASE_CAPACITY 16
    temp->songs_capacity = SONGS_BASE_CAPACITY;
    temp->song_count = 0;
    temp->songs = malloc(temp->songs_capacity * sizeof(Song));
    if (!temp->songs) {
        raise_error(ERR_MALLOC_NULL, "storage:load:songs");
        sqlite3_finalize(stmt);
        library_clear(temp);
        free(temp);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        storage_create_song(temp, sqlite3_column_int64(stmt, 0),
                            (const char*)sqlite3_column_text(stmt, 1),
                            (const char*)sqlite3_column_text(stmt, 2),
                            (const char*)sqlite3_column_text(stmt, 3),
                            (const char*)sqlite3_column_text(stmt, 4),
                            sqlite3_column_int64(stmt, 5),
                            (time_t)sqlite3_column_int64(stmt, 6));
    }
    sqlite3_finalize(stmt);

    // INSERTING PLAYLISTS
    query = "SELECT id, name FROM playlists";
    stmt = db_prepare(db, query);
    if (!stmt) {
        raise_error(ERR_SQLITE_FAILED, "storage:load:stmt");
        library_clear(temp);
        free(temp);
        return 0;
    };

#define PLAYLISTS_BASE_CAPACITY 16
    temp->playlists_capacity = PLAYLISTS_BASE_CAPACITY;
    temp->playlist_count = 0;
    temp->playlists = malloc(temp->playlists_capacity * sizeof(Playlist));
    if (!temp->playlists) {
        raise_error(ERR_MALLOC_NULL, "storage:load:playlists");
        sqlite3_finalize(stmt);
        library_clear(temp);
        free(temp);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        storage_create_playlist(temp, sqlite3_column_int64(stmt, 0),
                                (const char*)sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    // CREATING CONNECTION BEETWEEN SONGS AND PLAYLISTS
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

// finding the song by the id and returning the pointer
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

// creating and returning the song struct without adding it into the db
Song* storage_create_song(Library* library, size_t id, const char* path,
                          const char* title, const char* artist,
                          const char* album, size_t duration, time_t time) {
    if (!library) {
        raise_error(ERR_NULL_OBJECT, "storage:create_song:library");
        return NULL;
    }

    if (!path || !title || !artist || !album) {
        raise_error(ERR_NULL_OBJECT, "storage:create_song:args");
        return NULL;
    }

    for (size_t i = 0; i < library->song_count; i++) {
        if (strcmp(path, library->songs[i].path) == 0) {
            raise_error(ERR_SONG_ALREADY_EXISTS, "storage:create_song:song");
            return NULL;
        }
    }

    if (library->song_count + 1 >= library->songs_capacity) {
        library->songs_capacity *= 2;
        Song* temp =
            realloc(library->songs, library->songs_capacity * sizeof(Song));
        if (!temp) {
            raise_error(ERR_MALLOC_NULL, "storage:create_song:temp:realloc");
            return NULL;
        }

        library->songs = temp;
    }

    Song* song = &library->songs[library->song_count];
    song->id = id;
    song->path = copy_str(path);
    song->title = copy_str(title);
    song->artist = copy_str(artist);
    song->album = copy_str(album);
    song->duration = duration;
    song->time = time;

    library->song_count++;

    return song;
}

// adding the song struct into the database
int storage_add_song(sqlite3* db, Library* library, Song* song) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:add_song:db");
        return 0;
    }
    if (!library) {
        raise_error(ERR_NULL_OBJECT, "storage:add_song:library");
        return 0;
    }

    if (!song) {
        raise_error(ERR_NULL_OBJECT, "storage:add_song:song");
        return 0;
    }

    const char* query =
        "INSERT INTO songs (path, title, artist, album, duration, mtime) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = db_prepare(db, query);

    bind_str(stmt, 1, song->path);
    bind_str(stmt, 2, song->title);
    bind_str(stmt, 3, song->artist);
    bind_str(stmt, 4, song->album);
    bind_int(stmt, 5, song->duration);
    bind_time(stmt, 6, song->time);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        raise_error(ERR_SQLITE_FAILED, "storage:add_song:failed_to_add");
        return 0;
    }

    song->id = get_db_last_id(db);

    sqlite3_finalize(stmt);
    return 1;
}

// creating and returning the playlist struct without adding it into the db
Playlist* storage_create_playlist(Library* library, size_t id,
                                  const char* name) {
    if (!library) {
        raise_error(ERR_NULL_OBJECT, "storage:create_playlist:library");
        return NULL;
    }

    if (!name) {
        raise_error(ERR_NULL_OBJECT, "storage:create_playlist:name");
        return NULL;
    }

    for (size_t i = 0; i < library->playlist_count; i++) {
        if (strcmp(name, library->playlists[i].name) == 0) {
            raise_error(ERR_PLAYLIST_ALREADY_EXISTS,
                        "storage:create_playlist:name");
            return NULL;
        }
    }

    if (library->playlist_count + 1 >= library->playlists_capacity) {
        library->playlists_capacity *= 2;
        Playlist* temp = realloc(
            library->playlists, library->playlists_capacity * sizeof(Playlist));
        if (!temp) {
            raise_error(ERR_MALLOC_NULL,
                        "storage:create_playlist:temp:realloc");
            return NULL;
        }

        library->playlists = temp;
    }

    Playlist* playlist = &library->playlists[library->playlist_count];
    playlist->id = id;
    playlist->name = copy_str(name);
    playlist->songs = NULL;
    playlist->song_count = 0;

    library->playlist_count++;

    return playlist;
}

// adding the playlist struct into the database
int storage_add_playlist(sqlite3* db, Library* library, Playlist* playlist) {
    if (!db) {
        raise_error(ERR_NULL_OBJECT, "storage:add_playlist:db");
        return 0;
    }

    if (!library) {
        raise_error(ERR_NULL_OBJECT, "storage:add_playlist:library");
        return 0;
    }

    if (!playlist) {
        raise_error(ERR_NULL_OBJECT, "storage:add_playlistt:playlist");
        return 0;
    }

    const char*   query = "INSERT INTO playlists (name) VALUES (?)";
    sqlite3_stmt* stmt = db_prepare(db, query);

    bind_str(stmt, 1, playlist->name);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        raise_error(ERR_SQLITE_FAILED, "storage:add_playlist:stmt");
        return 0;
    }

    playlist->id = get_db_last_id(db);

    sqlite3_finalize(stmt);
    return 1;
}

int storage_playlist_add_song(sqlite3* db, Song* song, Playlist* playlist) {
    const char* query =
        "INSERT INTO playlist_songs (playlist_id, song_id, position) VALUES "
        "(?, ?, ?)";

    sqlite3_stmt* stmt = db_prepare(db, query);

    bind_int(stmt, 1, playlist->id);
    bind_int(stmt, 2, song->id);
    bind_int(stmt, 3, playlist->song_count);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        raise_error(ERR_SQLITE_FAILED, "storage:playlist_add_song:stmt");
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

// clearing the library fields
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

// clearing the playlists fields
void playlist_clear(Playlist* playlist) {
    if (!playlist) return;

    playlist->id = 0;
    playlist->song_count = 0;
    playlist->songs = NULL;

    if (playlist->name) {
        free(playlist->name);
    }
}

// clearing the song fields
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
}

// getting time_t struct for songs adding
time_t get_time() {
    time_t t;
    time(&t);
    return t;
}

// creating the full copy of string in heap
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

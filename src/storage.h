#ifndef STORAGE_H
#define STORAGE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "db_handler.h"
#include "error_handler.h"

typedef struct Song {
    size_t id;
    char*  path;
    char*  title;
    char*  artist;
    char*  album;
    size_t duration;
    time_t time;
} Song;

typedef struct Playlist {
    size_t id;
    char*  name;

    Song** songs;
    size_t song_count;
} Playlist;

typedef struct Library {
    Song*  songs;
    size_t song_count;

    Playlist* playlists;
    size_t    playlist_count;
} Library;

int storage_init(sqlite3** db);
int storage_close(sqlite3** db);

int storage_load(sqlite3* db, Library** library);

Song* find_song_by_id(Library* library, size_t id);

int storage_add_song(Library* library, const char* path, const char* title,
                     const char* artist, const char* album, size_t duration);

void library_clear(Library* library);
void playlist_clear(Playlist* playlist);
void song_clear(Song* song);

char* copy_str(const char* str);

#endif

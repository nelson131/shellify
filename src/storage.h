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
    size_t songs_capacity;

    Playlist* playlists;
    size_t    playlist_count;
    size_t    playlists_capacity;
} Library;

int storage_init(sqlite3** db);
int storage_close(sqlite3** db, Library** library);

int storage_load(sqlite3* db, Library** library);

Song* find_song_by_id(Library* library, size_t id);

Song* storage_create_song(Library* library, size_t id, const char* path,
                          const char* title, const char* artist,
                          const char* album, size_t duration, time_t time);
int   storage_add_song(sqlite3* db, Library* library, Song* song);

Playlist* storage_create_playlist(Library* library, size_t id,
                                  const char* name);
int storage_add_playlist(sqlite3* db, Library* library, Playlist* playlist);

void library_clear(Library* library);
void playlist_clear(Playlist* playlist);
void song_clear(Song* song);

char* copy_str(const char* str);

#endif

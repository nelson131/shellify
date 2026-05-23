#ifndef LIBRARY_H
#define LIBRARY_H

#include "logger.h"

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
    size_t capacity;
} Playlist;

typedef struct Library {
    Song** songs;
    size_t song_count;
    size_t songs_capacity;

    Playlist** playlists;
    size_t     playlist_count;
    size_t     playlists_capacity;
} Library;

// >>> main lib funcs
Library* lib_create(size_t sng_cap, size_t pl_cap);
void     lib_clear(Library* library);

// >>> songs funcs
Song* lib_new_sng(Library* library, size_t id, const char* path,
                  const char* title, const char* artist, const char* album,
                  size_t duration, time_t time);
Song* find_sng_by_id(Library* library, size_t id);

// >>> playlists funcs
Playlist* lib_new_plist(Library* library, size_t id, const char* name,
                        size_t cap);

// >>> clearing for lib elements
void lib_clear_sng(Song* sng);
void lib_clear_plist(Playlist* plist);

// >>> util funcs
char* copy_str(const char* str);

#endif

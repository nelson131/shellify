#include "library.h"

#include <stdlib.h>

#include "logger.h"

Library* lib_create(size_t sng_cap, size_t pl_cap) {
    Library* lib = calloc(1, sizeof(Library));
    if (!lib) {
        errlog(ERR_MALLOC_NULL, "lib:create:lib");
        return NULL;
    }

    lib->song_count = 0;
    lib->songs_capacity = sng_cap;
    lib->songs = malloc(sng_cap * sizeof(Song*));
    if (!lib->songs) {
        free(lib);
        errlog(ERR_MALLOC_NULL, "lib:create:songs");
        return NULL;
    }

    lib->playlist_count = 0;
    lib->playlists_capacity = pl_cap;
    lib->playlists = malloc(pl_cap * sizeof(Playlist*));
    if (!lib->playlists) {
        errlog(ERR_MALLOC_NULL, "lib:create:plists");
        free(lib->songs);
        free(lib);
        return NULL;
    }

    slog(INFO, "library has been created");
    return lib;
}

Song* lib_new_sng(Library* library, size_t id, const char* path,
                  const char* title, const char* artist, const char* album,
                  size_t duration, time_t time) {
    if (!library || !path || !title || !artist || !album) {
        errlog(ERR_NULL_OBJECT, "lib:new_song:args");
        return NULL;
    }

    for (size_t i = 0; i < library->song_count; i++) {
        if (strcmp(path, library->songs[i]->path) == 0) {
            alog(ERROR, path, "song already exists");
            return NULL;
        }
    }

    if (library->song_count + 1 >= library->songs_capacity) {
        size_t new_cap = library->songs_capacity * 2;
        Song** temp = realloc(library->songs, new_cap * sizeof(Song*));
        if (!temp) {
            errlog(ERR_MALLOC_NULL, "lib:new_song:temp");
            return NULL;
        }

        library->songs_capacity = new_cap;
        library->songs = temp;
        slog(WARNING, "library songs capacity has been increased");
    }

    Song* song = malloc(sizeof(Song));
    if (!song) {
        errlog(ERR_MALLOC_NULL, "lib:new_song:song");
        return NULL;
    }

    song->id = id;
    song->path = copy_str(path);
    song->title = copy_str(title);
    song->artist = copy_str(artist);
    song->album = copy_str(album);
    song->duration = duration;
    song->time = time;

    library->songs[library->song_count++] = song;

    alog(INFO, path, "song added to the memory");
    return song;
}

Song* find_sng_by_id(Library* library, size_t id) {
    if (!library) {
        errlog(ERR_NULL_OBJECT, "lib:find_sng_id:library");
        return NULL;
    }

    if (!library->song_count) {
        errlog(FAILED, "failed to find the song by id cause library is empty");
        return NULL;
    }

    for (size_t i = 0; i < library->song_count; i++) {
        if (library->songs[i]->id == id) {
            return library->songs[i];
        }
    }

    errlog(ERR_SONG_NOT_FOUND, "lib:find_sng_id:not_found");
    return NULL;
}

Playlist* lib_new_plist(Library* library, size_t id, const char* name,
                        size_t cap) {
    if (!library || !name) {
        errlog(ERR_NULL_OBJECT, "lib:new_plist:args");
        return NULL;
    }

    for (size_t i = 0; i < library->playlist_count; i++) {
        if (strcmp(name, library->playlists[i]->name) == 0) {
            errlog(ERR_PLAYLIST_ALREADY_EXISTS, "lib:new_plist:name");
            return NULL;
        }
    }

    if (library->playlist_count + 1 >= library->playlists_capacity) {
        size_t     new_cap = library->playlists_capacity * 2;
        Playlist** temp =
            realloc(library->playlists, new_cap * sizeof(Playlist*));
        if (!temp) {
            errlog(ERR_MALLOC_NULL, "lib:new_plist:temp");
            return NULL;
        }

        library->playlists_capacity = new_cap;
        library->playlists = temp;
        slog(WARNING, "playlists capacity has been increased");
    }

    Playlist* playlist = malloc(sizeof(Playlist));
    if (!playlist) {
        errlog(ERR_MALLOC_NULL, "lib:new_plist:playlist");
        return NULL;
    }

    playlist->id = id;
    playlist->name = copy_str(name);
    playlist->song_count = 0;
    playlist->capacity = cap;
    playlist->songs = malloc(cap * sizeof(Song*));
    if (!playlist->songs) {
        errlog(ERR_MALLOC_NULL, "lib:new_song:songs");
        free(playlist->name);
        free(playlist);
        return NULL;
    }

    library->playlists[library->playlist_count++] = playlist;

    alog(INFO, name, "playlist has been added in the memory");
    return playlist;
}

void lib_clear(Library* library) {
    if (!library) return;

    for (size_t i = 0; i < library->playlist_count; i++) {
        lib_clear_plist(library->playlists[i]);
        free(library->playlists[i]);
    }
    library->playlist_count = 0;
    free(library->playlists);

    for (size_t i = 0; i < library->song_count; i++) {
        lib_clear_sng(library->songs[i]);
        free(library->songs[i]);
    }
    library->song_count = 0;
    free(library->songs);

    slog(INFO, "library has been cleared");
}

void lib_clear_sng(Song* song) {
    if (!song) return;

    alog(INFO, song->path, "clearing the song");

    if (song->path) free(song->path);
    if (song->title) free(song->title);
    if (song->artist) free(song->artist);
    if (song->album) free(song->album);

    song->id = 0;
    song->duration = 0;
    song->time = 0;
}

void lib_clear_plist(Playlist* plist) {
    if (!plist) return;

    alog(INFO, plist->name, "clearing the playlist");

    if (plist->name) free(plist->name);
    if (plist->songs) free(plist->songs);

    plist->songs = NULL;
    plist->song_count = 0;
    plist->capacity = 0;
}

// creating the full copy of string in heap
char* copy_str(const char* str) {
    if (!str) {
        errlog(ERR_NULL_OBJECT, "storage:copy_str:str");
        return NULL;
    }

    char* s = malloc((strlen(str) + 1) * sizeof(char));
    if (!s) {
        errlog(ERR_MALLOC_NULL, "storage:copy_str:s");
        return NULL;
    }

    strcpy(s, str);
    return s;
}

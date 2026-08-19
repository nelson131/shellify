#include "controller.h"

#include "dl_queue.h"
#include "library.h"
#include "storage.h"

size_t last_playlist_id = 0;
size_t last_song_id = 0;

// >>> stg contoller

void add_song_tui(TUI* tui, Storage* stg) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "controller:add_song_tui:args");
        return;
    }

    TUI_InputForm* form = tui->input_form;
    if (!form || form->size < 4) {
        errlog(ERR_NULL_OBJECT, "controller:add_song_tui:input_form");
        return;
    }

    Playlist* playlist = stg->lib->playlists[tui->idx_plists];
    for (size_t i = 0; i < form->size; i++) {
        if (!form->values[i]) {
            errlog(ERR_NULL_OBJECT, "some value in input form is null");
            return;
        }
    }
    time_t t = time(NULL);

    int   seconds = audio_get_duration(form->values[0]);
    Song* song = lib_new_sng(stg->lib, 0, form->values[0], form->values[1],
                             form->values[2], form->values[3], seconds, t);
    if (song) {
        if (stg_add_sng(stg, song)) {
            if (stg_conn(stg, song, playlist))
                alog(INFO, song->path, "song has been added successfully");
        } else {
            errlog(ERR_SQLITE_FAILED, "failed to add the song");
        }
    } else {
        errlog(ERR_NULL_OBJECT, "add_song_tui:song");
    }
}

void add_song(TUI* tui, Storage* stg, const char* path, const char* title,
              const char* artist, const char* album) {
    if (!stg || !path || !title || !artist || !album) {
        errlog(ERR_NULL_OBJECT, "add_song:args");
        return;
    }

    Playlist* playlist = stg->lib->playlists[tui->idx_plists];
    time_t    t = time(NULL);

    int   seconds = audio_get_duration(path);
    Song* song =
        lib_new_sng(stg->lib, 0, path, title, artist, album, seconds, t);
    if (song) {
        if (stg_add_sng(stg, song)) {
            if (stg_conn(stg, song, playlist)) {
                alog(INFO, path, "song has been added successfully");
            }
        } else {
            errlog(ERR_SQLITE_FAILED, "failed to add the song");
        }
    } else {
        errlog(ERR_NULL_OBJECT, "add_song:song");
    }
}

void add_song_search(TUI* tui, Storage* stg, size_t idx) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "add_song_search:args");
        return;
    }

    SearchResult res = stg->sr_core->results[idx];

    char buf[512];
    snprintf(buf, sizeof(buf), "https://youtube.com/watch?v=%s", res.id);

    DLTask* task = dlq_task(stg->dlq, buf, res.title, res.artist, "Unknown");
    dlq_push(stg->dlq, task);
    free(task);
}

void add_song_import(TUI* tui, Storage* stg, size_t sr_idx, size_t import_idx) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "add_song_import:args");
        return;
    }

    SearchResult res = stg->sr_core->results[sr_idx];
    ImportEntry* entry = &stg->import_data->entries[import_idx];

    char buf[512];
    snprintf(buf, sizeof(buf), "https://youtube.com/watch?v=%s", res.id);

    DLTask* task =
        dlq_task(stg->dlq, buf, entry->title, entry->artist, entry->album);
    dlq_push(stg->dlq, task);
    free(task);
}

void add_plist(TUI* tui, Storage* stg) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "controller:add_plist:args");
        return;
    }

    if (!tui->input_form) {
        errlog(ERR_NULL_OBJECT, "controller:add_plist:input_form");
        return;
    }

    const char* plist_name = tui->input_form->values[0];
    Playlist*   playlist = lib_new_plist(stg->lib, 0, plist_name, 4);
    if (playlist) {
        if (stg_add_plist(stg, playlist)) {
            clear_input_form(tui);
        }
    } else {
        errlog(ERR_NULL_OBJECT, "failed to create the playlist in memory");
    }
}

void rem_song(TUI* tui, Storage* stg, Audio* audio) {
    if (!tui || !stg || !audio) {
        errlog(ERR_NULL_OBJECT, "controller:rem_song:args");
        return;
    }

    Playlist* plist = stg->lib->playlists[tui->idx_plists];
    if (plist->song_count == 0) return;
    if (last_song_id == tui->idx_songs) {
        audio_unload(audio);
    }
    if (!stg_rem_sng(stg, plist->songs[tui->idx_songs], plist)) {
        errlog(FAILED, "contoller:rem_song:stg");
        return;
    }

    lib_rem_sng_plist(plist, plist->songs[tui->idx_songs]);

    handle_idx(&tui->idx_songs);
}

void rem_song_abs(TUI* tui, Storage* stg, Audio* audio) {
    if (!tui || !stg || !audio) {
        errlog(ERR_NULL_OBJECT, "controller:rem_song:args");
        return;
    }

    Playlist* plist = stg->lib->playlists[tui->idx_plists];
    if (plist->song_count == 0) return;
    if (last_song_id == tui->idx_songs) {
        audio_unload(audio);
    }
    if (!stg_rem_sng_abs(stg, plist->songs[tui->idx_songs])) {
        errlog(FAILED, "controller:rem_song_abs:stg");
        return;
    }

    char* n = lib_rem_sng(stg->lib, plist->songs[tui->idx_songs]);
    if (n) {
        alog(INFO, n, "song has been deleted at all");
    }
    free(n);

    handle_idx(&tui->idx_songs);
}

void rem_plist(TUI* tui, Storage* stg) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "controller:rem_plist:args");
        return;
    }

    Playlist* plist = stg->lib->playlists[tui->idx_plists];
    if (stg->lib->playlist_count == 0) return;
    if (!stg_rem_plist(stg, plist)) {
        errlog(FAILED, "controller:rem_plist:stg");
        return;
    }

    char* n = lib_rem_plist(stg->lib, plist);
    if (n) {
        alog(INFO, n, "playlist has been deleted at all");
    }
    free(n);

    handle_idx(&tui->idx_plists);
}

void swap_songs(TUI* tui, Storage* stg, size_t from, size_t to) {
    if (!tui || !stg || from == to) return;

    Playlist* plist = stg->lib->playlists[tui->idx_plists];
    if (from >= plist->song_count || to >= plist->song_count) return;

    Song* sfrom = plist->songs[from];
    Song* sto = plist->songs[to];
    if (!sfrom || !sto) {
        errlog(ERR_NULL_OBJECT, "controller:swap_songs:songs");
        return;
    }

    slog(DEBUG, plist->songs[to]->title);
    slog(DEBUG, plist->songs[from]->title);

    stg_move_sng(stg, plist, from, to);

    plist->songs[from] = sto;
    plist->songs[to] = sfrom;

    slog(DEBUG, plist->songs[to]->title);
    slog(DEBUG, plist->songs[from]->title);
}

// >>> audio contoller

void handle_audio(int key, TUI* tui, Storage* stg, Audio* audio,
                  Config* config) {
    if (!tui || !audio) {
        errlog(ERR_NULL_OBJECT, "contoller:handle_audio:args");
        return;
    }

    if (audio->is_sound && key == config->keys.pause) {
        audio_pause(audio);
        return;
    }

    if (audio->is_sound && ma_sound_at_end(&audio->cur_sound)) {
        tui->idx_songs++;
        if (tui->idx_songs >=
            stg->lib->playlists[tui->idx_plists]->song_count) {
            tui->idx_songs = 0;
        }

        goto rock;
    }

    if (key == config->keys.select) {
        goto rock;
    }

    return;

rock:
    audio_play(
        audio,
        stg->lib->playlists[tui->idx_plists]->songs[tui->idx_songs]->path,
        (Vec){tui->idx_plists, tui->idx_songs});
    tui_up_progress_bar(tui);
}

void handle_next(TUI* tui, Storage* stg, Audio* audio, Config* config) {
    if (!tui || !stg || !audio || !config) return;

    Playlist* playlist = stg->lib->playlists[tui->idx_plists];
    if (playlist->song_count == 0) return;

    size_t idx = 0;
    if (config->player.shuffle) {
        idx = rand() % playlist->song_count;
    } else {
        idx = tui->idx_songs + 1;
        if (idx >= playlist->song_count) idx = 0;
    }

    tui->idx_songs = idx;

    Song* s = playlist->songs[idx];
    audio_play(audio, s->path, (Vec){tui->idx_plists, idx});
}

void handle_idx(size_t* idx) {
    if (!idx) return;

    if (*idx > 0) {
        *idx -= 1;
    } else {
        *idx = 0;
    }
}

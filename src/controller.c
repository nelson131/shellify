#include "controller.h"

#include "storage.h"

void add_song(TUI* tui, Storage* stg) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "controller:add_song:args");
        return;
    }

    if (!tui->input_form) {
        errlog(ERR_NULL_OBJECT, "controller:add_song:input_form");
        return;
    }

    Playlist*      playlist = stg->lib->playlists[tui->idx_plists];
    TUI_InputForm* form = tui->input_form;
    for (size_t i = 0; i < form->size; i++) {
        if (!form->values[i]) {
            errlog(ERR_NULL_OBJECT, "some value in input form is null");
            return;
        }
    }
    time_t t = time(NULL);

    Song* song = lib_new_sng(stg->lib, 0, form->values[0], form->values[1],
                             form->values[2], form->values[3], 200, t);
    if (song) {
        if (stg_add_sng(stg, song)) {
            if (stg_conn(stg, song, playlist))
                alog(INFO, song->path, "song has been added successfully");
        } else {
            errlog(ERR_SQLITE_FAILED, "failed to add the song");
        }
    } else {
        errlog(ERR_NULL_OBJECT, "add_song:song");
    }
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

void rem_song(TUI* tui, Storage* stg) {
    if (!tui || !stg) {
        errlog(ERR_NULL_OBJECT, "controller:rem_song:args");
        return;
    }

    Playlist* playlist = stg->lib->playlists[tui->idx_plists];
}

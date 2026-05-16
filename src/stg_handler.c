#include "stg_handler.h"

#include "storage.h"

void add_song(TUI* tui, sqlite3* db, Library* library) {
    if (!tui || !library || !tui->input_form) return;

    Playlist*      playlist = library->playlists[tui->idx_plists];
    TUI_InputForm* form = tui->input_form;
    for (size_t i = 0; i < form->size; i++) {
        if (!form->values[i]) {
            errlog(ERR_NULL_OBJECT, "some value in input form is null");
            return;
        }
    }
    time_t t = time(NULL);

    Song* song =
        storage_create_song(library, 0, form->values[0], form->values[1],
                            form->values[2], form->values[3], 200, t);
    if (song) {
        storage_playlist_add_song(db, song, playlist);
        slog(INFO, "song has been added successfully");
    } else {
        errlog(ERR_NULL_OBJECT, "add_song:song");
    }
}

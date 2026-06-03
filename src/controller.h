#ifndef STG_HANDLER_H
#define STG_HANDLER_H

#include <miniaudio/miniaudio.h>
#include <sqlite3.h>
#include <time.h>

#include "audio.h"
#include "library.h"
#include "logger.h"
#include "storage.h"
#include "tui.h"

extern size_t last_playlist_id;
extern size_t last_song_id;

// >>> stg contoller
void add_song(TUI* tui, Storage* stg);
void add_plist(TUI* tui, Storage* stg);

void rem_song(TUI* tui, Storage* stg, Audio* audio);
void rem_song_abs(TUI* tui, Storage* stg, Audio* audio);
void rem_plist(TUI* tui, Storage* stg);

// >>> audio contoller
void handle_audio(TUI* tui, Storage* stg, Audio* audio);
void handle_next(TUI* tui, Storage* stg, Audio* audio, Config* config);

// >>> utils
void handle_idx(size_t* idx);

#endif

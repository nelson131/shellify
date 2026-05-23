#ifndef STG_HANDLER_H
#define STG_HANDLER_H

#include <sqlite3.h>
#include <time.h>

#include "library.h"
#include "logger.h"
#include "storage.h"
#include "tui.h"

void add_song(TUI* tui, Storage* stg);
void add_plist(TUI* tui, Storage* stg);

void rem_song(TUI* tui, Storage* stg);
void rem_plist(TUI* tui, Storage* stg);

#endif

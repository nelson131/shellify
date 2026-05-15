#ifndef STG_HANDLER_H
#define STG_HANDLER_H

#include <sqlite3.h>
#include <time.h>

#include "storage.h"
#include "tui.h"

void add_song(TUI* tui, sqlite3* db, Library* library);
void add_playlist();

#endif

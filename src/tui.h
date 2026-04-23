#ifndef TUI_H
#define TUI_H

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "error_handler.h"

#define PREFIX_PLAYING " Now playing: "

extern char* separator;
extern char* line_now_playing;
extern char* song_name;

int  tui_init();
void tui_clear();

int create_header();
int create_welcome();

#endif

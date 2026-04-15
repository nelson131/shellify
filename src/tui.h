#ifndef TUI_H
#define TUI_H

#include <stdlib.h>

#include "config.h"
#include "error_handler.h"

static char* separator = NULL;

void tui_init();
void tui_clear();

void create_header();

#endif

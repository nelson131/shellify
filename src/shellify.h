#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <sys/ioctl.h>
#include <termios.h>

#include "buffer.h"
#include "config.h"
#include "input.h"
#include "storage.h"
#include "tui.h"

extern int      shellify_is_running;
extern sqlite3* db;

void shellify_init();
void shellify_destroy();

void shellify_draw();
void shellify_handle_input();

void shellify_stop();

#endif

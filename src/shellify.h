#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <sys/ioctl.h>
#include <termios.h>

#include "buffer.h"

static int shellify_is_running = 1;

void shellify_init();
void shellify_destroy();

void shellify_draw();
void shellify_handle_input();

#endif

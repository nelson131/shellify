#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdlib.h>

#include "logger.h"

char* clipboard_get();
char* clipboard_read(const char* cmd);

#endif

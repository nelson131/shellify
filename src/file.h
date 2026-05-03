#ifndef FILE_H
#define FILE_H

#include "config.h"
#include "error_handler.h"

size_t file_get_duration_sec(const char* path, Config* config);

#endif

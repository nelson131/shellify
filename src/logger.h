#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOG_FILE_PATH "%s/.config/shellify/%s"

typedef enum LogLevel { DEBUG, INFO, WARNING, ERROR } LogLevel;

extern FILE*       log_file;
extern const char* log_msgs[];

void logger_init(const char* file_name);
void logger_close();

void slog(LogLevel log_level, const char* msg);

#endif

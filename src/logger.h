#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOG_FILE_PATH "%s/.config/shellify/%s"

typedef enum LogLevel { DEBUG, INFO, WARNING, ERROR } LogLevel;

typedef enum ErrorCode {
    FAILED,
    NONE,
    SUCCESS,
    ERR_NULL_OBJECT,
    ERR_MALLOC_NULL,
    ERR_EMPTY_OBJECT,
    ERR_FILE_OPENING,
    ERR_CONFIG_LOAD,
    ERR_CONFIG_SAVE,
    ERR_SQLITE_OPEN,
    ERR_SQLITE_FAILED,
    ERR_SONG_NOT_FOUND,
    ERR_SONG_ALREADY_EXISTS,
    ERR_PLAYLIST_NOT_FOUND,
    ERR_PLAYLIST_ALREADY_EXISTS,
    ERR_AUDIO_INIT,
    ERR_AUDIO_FAILED,
    ERR_DL_FAILED
} ErrorCode;

extern FILE*       log_file;
extern const char* log_msgs[];
extern const char* err_msgs[];
extern size_t      logging;

void logger_init(const char* file_name, size_t log);
void logger_close();

void slog(LogLevel log_level, const char* msg);
void alog(LogLevel log_leve, const char* s1, const char* msg);
void errlog(ErrorCode err_code, const char* msg);

#endif

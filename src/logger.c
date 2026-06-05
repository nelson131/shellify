#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

FILE*       log_file = NULL;
const char* log_msgs[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
const char* err_msgs[] = {"FAILED",
                          "NONE",
                          "SUCCESS",
                          "ERR_NULL_OBJECT",
                          "ERR_MALLOC_NULL",
                          "ERR_EMPTY_OBJECT",
                          "ERR_FILE_OPENING",
                          "ERR_CONFIG_LOAD",
                          "ERR_CONFIG_SAVE",
                          "ERR_SQLITE_OPEN",
                          "ERR_SQLITE_FAILED",
                          "ERR_SONG_NOT_FOUND",
                          "ERR_SONG_ALREADY_EXISTS",
                          "ERR_PLAYLIST_NOT_FOUND",
                          "ERR_PLAYLIST_ALREADY_EXISTS"};
size_t**    logging = NULL;

void logger_init(const char* file_name, size_t* log) {
    if (!file_name) return;

#define BUF_BASE_SIZE 128
    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) return;

    const char* home = getenv("HOME");
    if (!home) {
        free(buf);
        return;
    }
    snprintf(buf, BUF_BASE_SIZE, LOG_FILE_PATH, home, file_name);

    log_file = fopen(buf, "a");
    if (!log_file) {
        log_file = stdout;
    }

    if (!logging) {
        logging = &log;
    }

    free(buf);
}

void logger_close() {
    if (log_file && log_file != stdout) {
        fclose(log_file);
        log_file = NULL;
    }
}

void slog(LogLevel log_level, const char* msg) {
    if (!log_file) return;

    const char* level_msg = log_msgs[log_level];
    if (!level_msg) return;

    char   tstr[64];
    time_t t = time(NULL);
    strftime(tstr, sizeof(tstr), "%c", localtime(&t));

    fprintf(log_file, "[%s]: {%s} -> %s\n", tstr, level_msg, msg);
}

void alog(LogLevel log_level, const char* s1, const char* msg) {
    if (!s1 || !msg) return;

    char buf[BUF_BASE_SIZE];
    snprintf(buf, BUF_BASE_SIZE, "arg: %s -> msg: %s", s1, msg);
    slog(log_level, buf);
}

void errlog(ErrorCode err_code, const char* msg) {
    if (!msg) return;

    const char* err_msg = err_msgs[err_code];

    char buf[BUF_BASE_SIZE];
    snprintf(buf, BUF_BASE_SIZE, "(%s) %s", err_msg, msg);
    slog(ERROR, buf);
}

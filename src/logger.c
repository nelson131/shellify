#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

FILE*       log_file = NULL;
const char* log_msgs[] = {"DEBUG", "INFO", "WARNING", "ERROR"};

void logger_init(const char* file_name) {
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

    free(buf);
}

void logger_close() {
    if (log_file && log_file != stdout) {
        fclose(log_file);
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

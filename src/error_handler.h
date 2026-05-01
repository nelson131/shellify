#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

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
    ERR_PLAYLIST_ALREADY_EXISTS
} ErrorCode;

void raise_error(ErrorCode code, const char* message);

#endif

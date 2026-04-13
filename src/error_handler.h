#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

typedef enum ErrorCode {
    FAILED,
    NONE,
    SUCCESS,
    ERR_NULL_OBJECT,
    ERR_MALLOC_NULL,
    ERR_FILE_OPENING,
    ERR_CONFIG_LOAD,
    ERR_CONFIG_SAVE
} ErrorCode;

void raise_error(ErrorCode code, const char* message);

#endif

#ifndef DL_QUEUE_H
#define DL_QUEUE_H

#include "logger.h"

typedef struct DLTask {
    char url[512];
    char title[64];
    char artist[64];
    char album[64];
} DLTask;

typedef struct DLQueue {
    DLTask* tasks;
    size_t  size;
    size_t  cap;
    size_t  front;
    size_t  rear;
} DLQueue;

typedef struct DLIterator {
    DLQueue** dlq;
    size_t    idx;
} DLIterator;

// DLQueue >>>
DLQueue* dlq_init();
void     dlq_close(DLQueue* q);

void    dlq_push(DLQueue* q, DLTask* t);
int     dlq_pop(DLQueue* q, DLTask* t);
DLTask* dlq_peek(DLQueue* q);

DLTask* dlq_task(DLQueue* q, const char* url, const char* title,
                 const char* artist, const char* album);
void    dlq_free_task(DLTask* t);

// DLIterator >>
DLIterator* dli_init(DLQueue* q);
void        dli_close(DLIterator* dli);

int     dli_has_next(DLIterator* dli);
DLTask* dli_next(DLIterator* dli);

#endif

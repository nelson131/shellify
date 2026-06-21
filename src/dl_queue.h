#ifndef DL_QUEUE_H
#define DL_QUEUE_H

#include "logger.h"

typedef struct DLTask {
    char   url[512];
    char   title[64];
    char   artist[64];
    char   album[64];
    size_t status;
} DLTask;

typedef struct DLQueue {
    DLTask* tasks;
    size_t  size;
    size_t  cap;
    size_t  front;
    size_t  rear;
} DLQueue;

DLQueue* dlq_init();
void     dlq_close(DLQueue* q);

void    push(DLQueue* q, DLTask* t);
int     pop(DLQueue* q, DLTask* t);
DLTask* peek(DLQueue* q);

#endif

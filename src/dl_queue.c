#include "dl_queue.h"

#include <stdlib.h>

DLQueue* dlq_init() {
    DLQueue* dlq = malloc(sizeof(DLQueue));
    if (!dlq) {
        errlog(ERR_MALLOC_NULL, "dl_queue:init:dlq");
        return NULL;
    }
#define DEF_QUEUE_CAP 8
    dlq->tasks = malloc(DEF_QUEUE_CAP * sizeof(DLTask));
    if (!dlq->tasks) {
        errlog(ERR_MALLOC_NULL, "dl_queue:init:tasks");
        free(dlq);
        return NULL;
    }

    dlq->size = 0;
    dlq->cap = DEF_QUEUE_CAP;
    dlq->front = 0;
    dlq->rear = 0;

    slog(INFO, "download queue has been init");
    return dlq;
}

void dlq_close(DLQueue* q) {
    if (!q) return;

    free(q->tasks);
    free(q);
    slog(INFO, "download queue has been closed");
}

void dlq_push(DLQueue* q, DLTask* t) {
    if (!q) return;

    if (q->size >= q->cap) {
        size_t  new_cap = q->cap * 2;
        DLTask* temp = malloc(new_cap * sizeof(DLTask));
        if (!temp) {
            errlog(ERR_MALLOC_NULL, "dl_queue:insert:temp");
            return;
        }

        for (size_t i = 0; i < q->size; i++) {
            size_t idx = (q->front + i) % q->cap;
            temp[i] = q->tasks[idx];
        }

        free(q->tasks);
        q->tasks = temp;
        q->cap = new_cap;
        q->front = 0;
        q->rear = q->size;
    }

    memcpy(&q->tasks[q->rear], t, sizeof(DLTask));
    q->rear = (q->rear + 1) % q->cap;
    q->size++;
}

int dlq_pop(DLQueue* q, DLTask* t) {
    if (!q || !t) return 0;
    if (q->size == 0) return 0;

    memcpy(t, &q->tasks[q->front], sizeof(DLTask));
    q->front = (q->front + 1) % q->cap;
    q->size--;

    return 1;
}

DLTask* dlq_peek(DLQueue* q) {
    if (!q || q->size == 0) return NULL;
    return &q->tasks[q->front];
}

DLTask* dlq_task(DLQueue* q, const char* url, const char* title,
                 const char* artist, const char* album) {
    DLTask* t = malloc(sizeof(DLTask));
    if (!t) return NULL;

    snprintf(t->url, sizeof(t->url), "%s", url);
    snprintf(t->title, sizeof(t->title), "%s", title);
    snprintf(t->artist, sizeof(t->artist), "%s", artist);
    snprintf(t->album, sizeof(t->album), "%s", album);

    return t;
}

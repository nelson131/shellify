#ifndef SEARCH_H
#define SEARCH_H

#define SR_LEN 512

#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <threads.h>

#include "logger.h"

typedef enum SearchState { SEARCH_STATE_FREE, SEARCH_STATE_BUSY } SearchState;

typedef struct SearchResult {
    char title[SR_LEN];
    char artist[SR_LEN];
    char id[SR_LEN];
} SearchResult;

typedef struct SearchCore {
    SearchState   state;
    size_t        size;
    SearchResult* results;
} SearchCore;

typedef struct SearchThread {
    SearchCore** core;
    const char*  query;
} SearchThread;

SearchCore* init_search_core();
void        close_search_core(SearchCore* core);

void  search_run(SearchCore* core, const char* query);
void* search_exec(void* thr);

#endif

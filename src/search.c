#include "search.h"

SearchCore* init_search_core() {
    SearchCore* core = malloc(sizeof(SearchCore));
    if (!core) {
        errlog(ERR_MALLOC_NULL, "search:init:core");
        return NULL;
    }

    core->state = SEARCH_STATE_FREE;
    core->size = 0;
    core->results = NULL;

    slog(INFO, "search core initialized");
    return core;
}

void search_run(SearchCore* core, const char* query) {
    if (!core || !query) {
        errlog(ERR_NULL_OBJECT, "search:run:query");
        return;
    }

    if (core->state != SEARCH_STATE_FREE) return;

    SearchThread* sr_thr = malloc(sizeof(SearchThread));
    if (!sr_thr) {
        errlog(ERR_MALLOC_NULL, "search:run:sr_thr");
        return;
    }

    core->state = SEARCH_STATE_BUSY;

    sr_thr->core = core;
    sr_thr->query = strdup(query);

    pthread_t thr;
    pthread_create(&thr, NULL, search_exec, sr_thr);
    pthread_detach(thr);

    alog(INFO, query, "search stared");
}

void close_search_core(SearchCore* core) {
    if (!core) return;

    free(core->results);
    free(core);

    slog(INFO, "search core closed");
}

void* search_exec(void* thr) {
    SearchThread* sr_thr = (SearchThread*)thr;
    if (!sr_thr) goto thread_exit;

#define BUF_BASE_SIZE 1024
    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) goto thread_exit;

    char* cmd = malloc(BUF_BASE_SIZE);
    if (!cmd) {
        free(buf);
        goto thread_exit;
    }

    snprintf(cmd, BUF_BASE_SIZE,
             "yt-dlp --print '%%(title)s|%%(id)s|%%(channel)s' 'ytsearch5:%s'",
             sr_thr->query);

    FILE* npipe = popen(cmd, "r");
    if (!npipe) {
        free(buf);
        free(cmd);
        goto thread_exit;
    }

    SearchCore* core = sr_thr->core;
    core->size = 5;
    core->results = malloc(core->size * sizeof(SearchResult));
    if (!core->results) {
        free(buf);
        free(cmd);
        pclose(npipe);
        goto thread_exit;
    }

    size_t idx = 0;
    while (idx < core->size && fgets(buf, BUF_BASE_SIZE, npipe) != NULL) {
        char* token = strtok(buf, "|");
        if (token != NULL) {
            strcpy(core->results[idx].title, token);
            token = strtok(NULL, "|");
            strcpy(core->results[idx].id, token);
            token = strtok(NULL, "|");
            strcpy(core->results[idx].artist, token);
        }
        idx++;
    }

    snprintf(buf, BUF_BASE_SIZE, "found by search thread: %zu", idx);
    slog(INFO, buf);

    pclose(npipe);
    free(cmd);
    free(buf);

thread_exit:
    sr_thr->core->state = SEARCH_STATE_FREE;
    free(sr_thr);
    pthread_exit(NULL);
}

#include "dl_handler.h"

void dlh_run(Storage* stg, Audio* audio, DLState* dl_state) {
    if (!stg || !stg->dlq || !dl_state) {
        errlog(ERR_NULL_OBJECT, "dl_handler:args");
        return;
    }

    DLQueue* q = stg->dlq;
    if (q->size == 0) return;

    if (*dl_state != DLSTATE_FREE) return;

    *dl_state = DLSTATE_BUSY;
    DLTask task;
    if (!pop(q, &task)) {
        *dl_state = DLSTATE_FREE;
        errlog(FAILED, "dl_handler:pop:queue");
        return;
    }

    DLThread dl_thread = {dl_state, &task, stg, audio};

    pthread_t thr;
    pthread_create(&thr, NULL, dlh_exec, &dl_thread);
    pthread_detach(thr);
}

void* dlh_exec(void* thr) {
    DLThread* dl_thread = (DLThread*)thr;
    if (!dl_thread) goto thread_exit;

#define BUF_BASE_SIZE 1024
    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) goto thread_exit;

    snprintf(buf, BUF_BASE_SIZE, "downloading started -> %s",
             dl_thread->task->url);
    slog(INFO, buf);

    snprintf(buf, BUF_BASE_SIZE,
             "yt-dlp --extract-audio --audio-format mp3 --audio-quality 0 "
             "-o \"%s/%%(title)s.%%(ext)s\" \"%s\" 2>&1",
             dl_thread->audio->music_dir, dl_thread->task->url);

    FILE* pipe = popen(buf, "r");
    if (!pipe) {
        errlog(ERR_FILE_OPENING, "dl_exec:popen");
        free(buf);
        goto thread_exit;
    }

    int status = pclose(pipe);
    if (status == 0) {
        slog(INFO, "downloading finished goood");
    } else {
        errlog(ERR_DL_FAILED, "dl_exec:status");
    }

    free(buf);

thread_exit:
    *dl_thread->state = DLSTATE_FREE;
    pthread_exit(NULL);
}

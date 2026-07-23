#include "dl_handler.h"

#include "storage.h"

void dlh_run(TUI* tui, Storage* stg, Audio* audio, DLState* dl_state,
             Config* config) {
    if (!tui || !stg || !stg->dlq || !audio || !dl_state) {
        errlog(ERR_NULL_OBJECT, "dl_handler:args");
        return;
    }

    if (!config->general.enable_dlq) return;

    DLQueue* q = stg->dlq;
    if (q->size == 0) return;
    if (*dl_state != DLSTATE_FREE) return;

    DLThread* dl_thread = malloc(sizeof(DLThread));
    if (!dl_thread) {
        errlog(ERR_MALLOC_NULL, "dl_handler:thread");
        return;
    }

    *dl_state = DLSTATE_BUSY;
    dl_thread->state = dl_state;
    dl_thread->tui = tui;
    dl_thread->stg = stg;
    dl_thread->audio = audio;

    if (!dlq_pop(q, &dl_thread->task)) {
        *dl_state = DLSTATE_FREE;
        free(dl_thread);
        errlog(FAILED, "dl_handler:pop:queue");
        return;
    }

    pthread_t thr;
    pthread_create(&thr, NULL, dlh_exec, dl_thread);
    pthread_detach(thr);
}

void* dlh_exec(void* thr) {
    DLThread* dl_thread = (DLThread*)thr;
    if (!dl_thread) goto thread_exit;

#define BUF_BASE_SIZE 1024
    char* buf = malloc(BUF_BASE_SIZE);
    if (!buf) goto thread_exit;

    char* cmd = malloc(BUF_BASE_SIZE);
    if (!cmd) {
        free(buf);
        goto thread_exit;
    }

    snprintf(cmd, BUF_BASE_SIZE,
             "yt-dlp --get-filename -o \"%%(title)s.mp3\" \"%s\"",
             dl_thread->task.url);

    char  filename[256] = {0};
    FILE* npipe = popen(cmd, "r");
    if (npipe) {
        if (fgets(filename, sizeof(filename), npipe) != NULL) {
            filename[strcspn(filename, "\n")] = '\0';
        }
        pclose(npipe);
    }

    if (strlen(filename) == 0) {
        strncpy(filename, "unknown.mp3", sizeof(filename));
    }

    snprintf(cmd, BUF_BASE_SIZE,
             "yt-dlp --extract-audio --audio-format mp3 --audio-quality 0 "
             "-o \"%s/%%(title)s.%%(ext)s\" \"%s\" 2>&1",
             dl_thread->audio->music_dir, dl_thread->task.url);

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        errlog(ERR_FILE_OPENING, "dl_exec:popen");
        free(cmd);
        free(buf);
        goto thread_exit;
    }

    char garbage[128];
    while (fgets(garbage, sizeof(garbage), pipe) != NULL) {
    }

    int status = pclose(pipe);
    if (status == 0) {
        slog(INFO, "downloading finished goood");
        DLTask* task = &dl_thread->task;
        add_song(dl_thread->tui, dl_thread->stg, filename, task->title,
                 task->artist, task->album);
    } else {
        errlog(ERR_DL_FAILED, "dl_exec:status");
    }

    free(buf);
    free(cmd);

thread_exit:
    *dl_thread->state = DLSTATE_FREE;
    free(dl_thread);
    pthread_exit(NULL);
}

void dlh_save_stg(Storage* stg) {
    if (!stg || !stg->dlq) return;

    if (stg_clear_dlq(stg) != 1) {
        errlog(ERR_DL_FAILED, "dlh:save_stg:clear_dlq");
        return;
    }

    DLTask task;
    size_t count = stg->dlq->size;
    for (size_t i = 0; i < count; i++) {
        if (dlq_pop(stg->dlq, &task) != 1) {
            errlog(ERR_DL_FAILED, "dlh:save_stg:pop");
            return;
        }

        if (stg_add_dlq_task(stg, &task) != 1) {
            errlog(ERR_DL_FAILED, "dlh:save_stg:add_dlq_task");
            return;
        }
    }

    slog(INFO, "dlq has been saved");
}

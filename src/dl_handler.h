#ifndef DL_HANDLER_H
#define DL_HANDLER_H

#include <pthread.h>
#include <stdatomic.h>
#include <threads.h>

#include "audio.h"
#include "dl_queue.h"
#include "logger.h"
#include "storage.h"

typedef enum DLState { DLSTATE_BUSY, DLSTATE_FREE } DLState;

typedef struct DLThread {
    DLState* state;
    DLTask*  task;
    Storage* stg;
    Audio*   audio;
} DLThread;

void  dlh_run(Storage* stg, Audio* audio, DLState* dl_state);
void* dlh_exec(void* thr);

#endif

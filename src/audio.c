#include "audio.h"

void audio_init(Audio** audio) {
    if (audio && *audio) return;
    *audio = malloc(sizeof(Audio));
    if (!*audio) {
        errlog(ERR_MALLOC_NULL, "audio:init:struct");
        return;
    }

    ma_result res = ma_engine_init(NULL, &(*audio)->engine);
    if (res != MA_SUCCESS) {
        errlog(ERR_AUDIO_INIT, "audio:init:engine");
        free(audio);
        return;
    }
    (*audio)->is_init = 1;
    slog(INFO, "audio has been init");
}

void audio_close(Audio** audio) {
    if (!audio) return;

    if ((*audio)->is_sound) {
        ma_sound_uninit(&(*audio)->cur_sound);
    }

    if ((*audio)->is_init) {
        ma_engine_uninit(&(*audio)->engine);
    }

    free(*audio);
    *audio = NULL;
    slog(INFO, "audio has been closed");
}

void audio_update(Audio* audio, float volume) {
    if (!audio) return;
    if (volume < 0 || volume > 1.0) return;

    ma_engine_set_volume(&audio->engine, volume);
}

void audio_play(Audio* audio, const char* path) {
    if (!audio) {
        errlog(ERR_NULL_OBJECT, "audio:play:audio");
        return;
    }
    if (!path) {
        errlog(ERR_NULL_OBJECT, "audio:play:path");
        return;
    }

    if (audio->is_sound) {
        ma_sound_stop(&audio->cur_sound);
        ma_sound_uninit(&audio->cur_sound);
        audio->is_sound = 0;
    }

    ma_result res = ma_sound_init_from_file(&audio->engine, path, 0, NULL, NULL,
                                            &audio->cur_sound);
    if (res != MA_SUCCESS) {
        alog(ERROR, path, "failed to play audio");
        return;
    }

    audio->is_sound = 1;
    ma_sound_start(&audio->cur_sound);
    alog(INFO, path, "playing the song");
}

void audio_pause(Audio* audio) {
    if (!audio || !audio->is_sound) return;

    if (ma_sound_is_playing(&audio->cur_sound)) {
        ma_sound_stop(&audio->cur_sound);
        slog(INFO, "audio sound stopped");
    } else {
        ma_sound_start(&audio->cur_sound);
        slog(INFO, "audio sound resumed");
    }
}

void audio_stop(Audio* audio) {
    if (!audio || !audio->is_sound) return;

    ma_sound_stop(&audio->cur_sound);
    slog(INFO, "audio sound stopped");
}

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
        free(*audio);
        return;
    }
    (*audio)->is_init = 1;
    (*audio)->is_sound = 0;
    (*audio)->is_stopped = 0;

    (*audio)->sng_idx = -1;
    (*audio)->plist_idx = -1;

#define MUSIC_DIR_SIZE strlen(MUSIC_DIR_AU) + 64
    (*audio)->music_dir = malloc(MUSIC_DIR_SIZE);
    const char* home = getenv("HOME");
    snprintf((*audio)->music_dir, MUSIC_DIR_SIZE, MUSIC_DIR_AU, home);

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

    if ((*audio)->music_dir) {
        free((*audio)->music_dir);
    }

    free(*audio);
    *audio = NULL;
    slog(INFO, "audio has been closed");
}

void audio_volume(Audio* audio, Config* config) {
    if (!audio) return;

    ma_engine_set_volume(&audio->engine, config->player.volume);
}

void audio_play(Audio* audio, const char* path, Vec idxes) {
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

    char buf[128];
    snprintf(buf, sizeof(buf), "%s%s", audio->music_dir, path);
    ma_result res = ma_sound_init_from_file(&audio->engine, buf, 0, NULL, NULL,
                                            &audio->cur_sound);
    if (res != MA_SUCCESS) {
        alog(ERROR, path, "failed to play audio");
        return;
    }

    audio->plist_idx = idxes.x;
    audio->sng_idx = idxes.y;

    audio->is_sound = 1;
    audio->is_stopped = 0;
    ma_sound_start(&audio->cur_sound);
    alog(INFO, path, "playing the song");
}

void audio_pause(Audio* audio) {
    if (!audio || !audio->is_sound) return;

    if (ma_sound_is_playing(&audio->cur_sound)) {
        ma_sound_stop(&audio->cur_sound);
        audio->is_stopped = 1;
        slog(INFO, "audio sound stopped");
    } else {
        ma_sound_start(&audio->cur_sound);
        audio->is_stopped = 0;
        slog(INFO, "audio sound resumed");
    }
}

void audio_stop(Audio* audio) {
    if (!audio || !audio->is_sound) return;

    ma_sound_stop(&audio->cur_sound);
    slog(INFO, "audio sound stopped");
}

void audio_unload(Audio* audio) {
    if (!audio || !audio->is_sound) return;

    ma_sound_stop(&audio->cur_sound);
    ma_sound_uninit(&audio->cur_sound);
    audio->is_sound = 0;
}

int audio_is_ended(Audio* audio) {
    if (!audio || !audio->is_init || !audio->is_sound) return 0;
    return ma_sound_at_end(&audio->cur_sound);
}

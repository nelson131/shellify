#ifndef AUDIO_H
#define AUDIO_H

#include <miniaudio/miniaudio.h>
#include <stdlib.h>

#include "logger.h"

typedef struct Audio {
    ma_engine engine;
    int       is_init;

    ma_sound cur_sound;
    int      is_sound;
} Audio;

void audio_init(Audio** audio);
void audio_close(Audio** audio);

void audio_update(Audio* audio, float volume);

void audio_play(Audio* audio, const char* path);
void audio_pause(Audio* audio);
void audio_stop(Audio* audio);

#endif

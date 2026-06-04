#ifndef AUDIO_H
#define AUDIO_H

#include <miniaudio/miniaudio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logger.h"

#define MUSIC_DIR_AU "%s/Music/shellify/"

typedef struct Audio {
    ma_engine engine;
    int       is_init;

    ma_sound cur_sound;
    int      is_sound;
    int      is_stopped;

    char* music_dir;
} Audio;

void audio_init(Audio** audio);
void audio_close(Audio** audio);

void audio_volume(Audio* audio, Config* config);

void audio_play(Audio* audio, const char* path);
void audio_pause(Audio* audio);
void audio_stop(Audio* audio);
void audio_unload(Audio* audio);

int audio_is_ended(Audio* audio);

#endif

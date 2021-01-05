#include "game.h"
#include "asset/wav.h"

#include <math.h>
#include <stdio.h>

#define Pi32 3.14159265359f
static float sine = 0.0f;

Sound test_sound = {};

extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    
}

int position;

extern "C" __declspec(dllexport) GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    if (!test_sound.samples)
    {
        void* data = platform->load_file("C:\\projects\\dababy.wav");
        test_sound = load_wav(data);
    }

    int16_t* at = sound_buffer->samples;

    for (auto sindex = 0; sindex < sound_buffer->sample_count; ++sindex)
    {
        *at++ = test_sound.samples[position];
        *at++ = test_sound.samples[position + 1];

        position += 2;
        if (position > test_sound.sample_count) position = 0;
    }
}
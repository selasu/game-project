#include "game.h"

#include <math.h>

#define Pi32 3.14159265359f
static float sine = 0.0f;

extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    
}

extern "C" __declspec(dllexport) GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    int16_t volume  = 3000;
    int32_t tone_hz = 512;
    int32_t wave_period = sound_buffer->samples_per_second / tone_hz;

    int16_t* output = sound_buffer->samples;
    for (int sindex = 0; sindex < sound_buffer->sample_count; ++sindex)
    {
        float sine_value = sinf(sine);
        int16_t value = (int16_t)(sine_value * volume);

        *output++ = value;
        *output++ = value;

        sine += 2.0f * Pi32 * 1.0f / (float)wave_period;
        if (sine > 2.0f * Pi32)
        {
            sine -= 2.0f * Pi32;
        }
    }
}
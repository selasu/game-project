#include <stdint.h>

struct SoundBuffer
{
    int32_t samples_per_second;
    int32_t sample_count;

    int16_t* samples;
};

#define GAME_UPDATE_AND_RENDER(name) void name(int a)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_t);

#define GAME_GET_SOUND_SAMPLES(name) void name(SoundBuffer* sound_buffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_t);
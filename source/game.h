#include "platform/platform.h"
#include "types.h"
#include "asset/wav.h"

#include <stdint.h>

struct SoundBuffer
{
    int32_t samples_per_second;
    int32_t sample_count;

    int16_t* samples;
};

struct PlayingSound
{
    Sound   sound_data;
    int32_t position;

    v2   volume;
    bool looping;
    bool active;

    PlayingSound* next;
};

struct Audio
{
    v2 master_volume;

    PlayingSound* first_playing;
    PlayingSound* first_free;
};

struct Game
{
    bool initialised;

    Audio* audio;
};

static char* game_functions[] =
{
    "game_update_and_render",
    "game_get_sound_samples"
};

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory* memory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_t);

#define GAME_GET_SOUND_SAMPLES(name) void name(GameMemory* memory, SoundBuffer* sound_buffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_t);
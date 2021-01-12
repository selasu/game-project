#include "types.h"
#include "wav.h"
#include "platform/platform.h"

struct SoundBuffer
{
    i32 samples_per_second;
    i32 sample_count;

    i16* samples;
};

struct PlayingSound
{
    LoadedSound sound_data;
    i32         samples_played;

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
#include "game.h"
#include "asset/wav.h"

struct PlayingSound
{
    Sound   sound_data;
    int32_t position;

    float volume;
    bool  looping;
    bool  active;
};

extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    
}

PlayingSound sound = {};
bool loaded        = false;

extern "C" __declspec(dllexport) GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    if (!loaded)
    {
        loaded = true;

        sound.sound_data = load_wav(platform->load_file("C:\\projects\\benny.wav"));
        sound.volume     = 1.0f;
        sound.active     = true;
        sound.looping    = false;
    }

    int16_t* at = sound_buffer->samples;
    for (auto sindex = 0; sindex < sound_buffer->sample_count; ++sindex)
    {
        float lsample = 0;
        float rsample = 0;

        if (sound.active)
        {
            int32_t sample = (int32_t)(sound.position * sound.sound_data.channel_count);

            lsample = sound.sound_data.samples[sample] * sound.volume;
            rsample = sound.sound_data.samples[sample + (sound.sound_data.channel_count - 1)] * sound.volume;

            sound.position += 1;
            if (sound.position >= sound.sound_data.sample_count)
            {
                if (sound.looping)
                {
                    sound.position = 0;
                }
                else
                {
                    sound.active = false;
                }
            }
        }

        *at++ = (int16_t)lsample;
        *at++ = (int16_t)rsample;
    }
}
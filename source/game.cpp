#include "game.h"

#include <stdio.h>

extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    
}

extern "C" __declspec(dllexport) GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    PlatformAPI* platform = &memory->platform_api;

    Game* game_state = memory->game_state;
    if (!game_state)
    {
        game_state = memory->game_state = (Game*)platform->allocate_memory(sizeof(game_state));

        PlayingSound* sound = (PlayingSound*)platform->allocate_memory(sizeof(PlayingSound));
        sound->sound_data = load_wav(platform->load_file("C:\\projects\\test.wav"));
        sound->volume     = v2{1.0f, 1.0f};
        sound->active     = true;
        sound->looping    = false;

        PlayingSound* sound2 = (PlayingSound*)platform->allocate_memory(sizeof(PlayingSound));
        sound2->sound_data = load_wav(platform->load_file("C:\\projects\\benny.wav"));
        sound2->volume     = v2{1.0f, 1.0f};
        sound2->active     = true;
        sound2->looping    = false;

        sound->next  = sound2;
        sound2->next = 0;

        game_state->audio = (Audio*)platform->allocate_memory(sizeof(game_state->audio));
        game_state->audio->master_volume = v2{1.0f, 1.0f};
        game_state->audio->first_playing = sound;
    }

    Audio* audio = game_state->audio;
    int16_t* at  = sound_buffer->samples;

    for (int i = 0; i < sound_buffer->sample_count; ++i)
    {
        int16_t lsample = 0;
        int16_t rsample = 0;

        for (PlayingSound** sound_ptr = &audio->first_playing; *sound_ptr;)
        {
            PlayingSound* sound = *sound_ptr;

            int32_t sample = (int32_t)(sound->position * sound->sound_data.channel_count);

            lsample += (int16_t)(sound->sound_data.samples[sample] * sound->volume.x);
            rsample += (int16_t)(sound->sound_data.samples[sample + sound->sound_data.channel_count - 1] * sound->volume.y);

            ++sound->position;
            if (sound->position >= sound->sound_data.sample_count)
            {
                if (sound->looping)
                {
                    sound->position = 0;
                }
                else
                {
                    *sound_ptr = sound->next;
                    sound->next = audio->first_free;
                    audio->first_free = sound;
                }
            }
            else
            {
                sound_ptr = &sound->next;
            }
        }

        *at++ = (int16_t)(lsample * audio->master_volume.x);
        *at++ = (int16_t)(rsample * audio->master_volume.y);
    }
}
#include "game.h"

extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    
}

extern "C" __declspec(dllexport) GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    PlatformAPI* platform = &memory->platform_api;
    Game* game_state      = memory->game_state;

    if (!game_state)
    {
        // TODO(selina): Replace all this with a more robust system for initialising memory and loading assets

        game_state = memory->game_state = (Game*)platform->allocate_memory(sizeof(game_state));

        PlayingSound* sound = (PlayingSound*)platform->allocate_memory(sizeof(PlayingSound));
        sound->sound_data = load_wav(platform->load_file("C:\\projects\\game\\resources\\acapella.wav"));
        sound->volume     = v2{1.0f, 1.0f};
        sound->active     = true;
        sound->looping    = false;

        PlayingSound* sound2 = (PlayingSound*)platform->allocate_memory(sizeof(PlayingSound));
        sound2->sound_data = load_wav(platform->load_file("C:\\projects\\game\\resources\\instrumental.wav"));
        sound2->volume     = v2{0.25f, 0.25f};
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

    // NOTE(selina): This is just testing the game code reloading
    audio->first_playing->volume = v2{1.0f, 1.0f}; // 100% volume - acapella
    // audio->first_playing->volume = v2{0.0f, 0.0f}; // 0% volume - acapella

    for (int i = 0; i < sound_buffer->sample_count; ++i)
    {
        int16_t lsample = 0;
        int16_t rsample = 0;

        for (PlayingSound** sound_ptr = &audio->first_playing; *sound_ptr;)
        {
            PlayingSound* sound = *sound_ptr;

            if (!sound->active) continue;

            int32_t sample = (int32_t)(sound->samples_played * sound->sound_data.channel_count);

            lsample += (int16_t)(sound->sound_data.samples[sample] * sound->volume.x);
            rsample += (int16_t)(sound->sound_data.samples[sample + sound->sound_data.channel_count - 1] * sound->volume.y);

            ++sound->samples_played;
            if (sound->samples_played >= sound->sound_data.sample_count)
            {
                if (sound->looping)
                {
                    sound->samples_played = 0;
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
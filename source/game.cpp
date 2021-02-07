#include "game.h"

#include "png.h"

extern "C" __declspec(dllexport) GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    PlatformAPI* platform = &memory->platform_api;
    Game* game_state      = memory->game_state;

    if (!game_state)
    {
        // TODO(selina): Replace all this with a more robust system for initialising memory and loading assets

        LoadedPNG png = load_png(platform->load_file("C:\\projects\\game\\resources\\photoshop test.png"));

        game_state = memory->game_state = (Game*)platform->allocate_memory(sizeof(game_state));

        PlayingSound* sound = (PlayingSound*)platform->allocate_memory(sizeof(PlayingSound));
        sound->sound_data = load_wav(platform->load_file("C:\\projects\\game\\resources\\acapella.wav").data);
        sound->volume     = v2{1.0f, 1.0f};
        sound->active     = true;
        sound->looping    = false;

        PlayingSound* sound2 = (PlayingSound*)platform->allocate_memory(sizeof(PlayingSound));
        sound2->sound_data = load_wav(platform->load_file("C:\\projects\\game\\resources\\instrumental.wav").data);
        sound2->volume     = v2{0.25f, 0.25f};
        sound2->active     = true;
        sound2->looping    = false;

        sound->next  = sound2;
        sound2->next = 0;

        game_state->audio = (Audio*)platform->allocate_memory(sizeof(game_state->audio));
        game_state->audio->master_volume = v2{1.0f, 1.0f};
        game_state->audio->first_playing = sound;
    }

    push_quad(v3{-0.5f, -0.5f, 0.0f}, v3{-0.5f,  0.5f, 0.0f}, v3{0.5f,  0.5f, 0.0f}, v3{0.5f, -0.5f, 0.0f}, render_state);
    push_quad(v3{-0.9f, -0.9f, 0.0f}, v3{-0.9f,  -0.8f, 0.0f}, v3{-0.8f,  -0.8f, 0.0f}, v3{-0.8f, -0.9f, 0.0f}, render_state);
}

extern "C" __declspec(dllexport) GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    Game* game_state = memory->game_state;

    Audio* audio = game_state->audio;
    i16* at      = sound_buffer->samples;

    for (i32 i = 0; i < sound_buffer->sample_count; ++i)
    {
        i16 lsample = 0;
        i16 rsample = 0;

        for (PlayingSound** sound_ptr = &audio->first_playing; *sound_ptr;)
        {
            PlayingSound* sound = *sound_ptr;

            // NOTE(selina): Don't play inactive sounds
            if (!sound->active) 
            {
                sound_ptr = &sound->next;
                continue;
            }

            i32 sample = (i32)(sound->samples_played * sound->sound_data.channel_count);

            lsample += (i16)(sound->sound_data.samples[sample] * sound->volume.x);
            rsample += (i16)(sound->sound_data.samples[sample + sound->sound_data.channel_count - 1] * sound->volume.y);

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

        *at++ = (i16)(lsample * audio->master_volume.x);
        *at++ = (i16)(rsample * audio->master_volume.y);
    }
}
#include "engine/platform/os.h"
#include "engine/platform/render/ogl.h"
#include "engine/asset/wav.h"

#include <stdio.h>
#include <math.h>

#define PI 3.14159265359f

void output_sound(engine::SoundBuffer* sound_buffer)
{
    static float tSine  = 0;
    int16_t tone_volume = 3000;
    int wave_period     = sound_buffer->samples_per_second / 400;

    int16_t* output = sound_buffer->samples;
    for (int _ = 0; _ < sound_buffer->sample_count; ++_)
    {
        int16_t value = (int16_t)(sinf(tSine) * tone_volume);

        *output++ = value;
        *output++ = value;

        tSine += (2 * PI) / 1.0f / (float)wave_period;
        if (tSine > 2 * PI)
        {
            tSine -= 2 * PI;
        }
    }
}

int main(int argc, char** argv)
{
    using namespace engine;

    Config cfg;
    cfg.title  = "Test";
    cfg.width  = 720;
    cfg.height = 480;

    cfg.bytes_per_sample   = sizeof(int16_t) * 2;
    cfg.samples_per_second = 48000;

    cfg.ogl_version_major = 3;
    cfg.ogl_version_minor = 3;

    cfg.thread_count = 8;

    auto platform_data = os_create_context(cfg);
    if (!platform_data)
    {
        printf("Error: %u\n", os_poll_error());
        return -1;
    }

    auto running = true;
    auto event_handler = [&running](Event e) {
        switch(e.event_type)
        {
            case EventType::QUIT: 
            {
                running = false;
            } break;

            case EventType::RESIZE:
            {
                glViewport(0, 0, e.width, e.height);
            } break;

            default:
                break;
        }
    };
    
    // NOTE(selina): Temp asset loading for example
    auto sound = load_wav("C:/projects/dababy.wav");

    while (running)
    {
        os_process_events(platform_data->context, event_handler);

        glClearColor(77.0f / 255.0f, 0.0f, 153.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        output_sound(&platform_data->sound_buffer);

        os_update_context(platform_data);
    }

    os_delete_context(platform_data);
    
    return 0;
}
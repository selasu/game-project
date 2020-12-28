#include "engine/platform/os.h"
#include "engine/platform/render/ogl.h"
#include "engine/asset/wav.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    using namespace engine;

    load_wav("C:/projects/dababy.wav");

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

    while (running)
    {
        os_process_events(platform_data->context, event_handler);

        glClearColor(77.0f / 255.0f, 0.0f, 153.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        os_update_context(platform_data);
    }

    os_delete_context(platform_data);
    
    return 0;
}
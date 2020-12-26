#include "engine/platform/os.h"
#include "engine/platform/render/ogl.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    using namespace engine;

    OSFile file = os_read_file("c:/projects/test.txt");
    if (file.content)
    {
        os_write_file("c:/projects/test.out", file.size, file.content);\
        os_free_file(file.content);
    }
    else
    {
        printf("Uh oh\n");
    }

    Config cfg;
    cfg.title  = "Test";
    cfg.width  = 720;
    cfg.height = 480;
    cfg.ogl_version_major = 3;
    cfg.ogl_version_minor = 3;

    auto result   = os_create_context(cfg);
    auto* context = std::get_if<Context*>(&result);
    if (!context)
    {
        auto e = std::get<PlatformError>(result);
        printf("Error: %d %d\n", e.code, e.line);
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
        os_process_events(*context, event_handler);
        os_update_context(*context);

        glClearColor(77.0f / 255.0f, 0.0f, 153.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    os_delete_context(*context);
    
    return 0;
}
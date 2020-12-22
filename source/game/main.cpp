#include "engine/platform/os.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    engine::Config cfg;
    cfg.title  = "Test";
    cfg.width  = 720;
    cfg.height = 480;
    cfg.ogl_version_major = 3;
    cfg.ogl_version_minor = 3;

    auto result   = engine::os_create_context(cfg);
    auto* context = std::get_if<engine::Context*>(&result);
    if (!context)
    {
        auto e = std::get<engine::PlatformError>(result);
        printf("Error: %d %d\n", e.code, e.line);
        return -1;
    }

    auto running = true;
    auto event_handler = [&running](engine::Event e) {
        switch(e.event_type)
        {
            case engine::EventType::QUIT: 
            {
                running = false;
            } break;
            default:
                break;
        }
    };

    while (running)
    {
        engine::os_process_events(*context, event_handler);
        engine::os_update_context(*context);
    }

    engine::os_delete_context(*context);
    
    return 0;
}
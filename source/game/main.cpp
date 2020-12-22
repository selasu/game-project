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

    for (int i = 0; i < 100000000; i++) engine::os_update_context(*context);
    engine::os_delete_context(*context);
    
    return 0;
}